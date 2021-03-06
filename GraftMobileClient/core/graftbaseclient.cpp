#include "accountmodelserializator.h"
#include "barcodeimageprovider.h"
#include "api/graftgenericapi.h"
#include "quickexchangemodel.h"
#include "graftclienttools.h"
#include "graftbaseclient.h"
#include "qrcodegenerator.h"
#include "accountmanager.h"
#include "currencymodel.h"
#include "currencyitem.h"
#include "accountmodel.h"
#include "config.h"

#include <QGuiApplication>
#include <QStandardPaths>
#include <QHostAddress>
#include <QQmlContext>
#include <QTimerEvent>
#include <QClipboard>
#include <QQmlEngine>
#include <QSettings>
#include <QFileInfo>
#include <QDir>

static const QString scBarcodeImageProviderID("barcodes");
static const QString scQRCodeImageID("qrcode");
static const QString scAddressQRCodeImageID("address_qrcode");
static const QString scCoinAddressQRCodeImageID("coin_address_qrcode");
static const QString scProviderScheme("image://%1/%2");
static const QString scAccountModelDataFile("accountList.dat");
static const QString scSettingsDataFile("Settings.ini");

GraftBaseClient::GraftBaseClient(QObject *parent)
    : QObject(parent)
    ,mImageProvider(nullptr)
    ,mQRCodeEncoder(new QRCodeGenerator())
    ,mClientSettings(nullptr)
    ,mAccountModel(nullptr)
    ,mCurrencyModel(nullptr)
    ,mQuickExchangeModel(nullptr)
    ,mBalanceTimer(-1)
    ,mAccountManager(new AccountManager())
{
    initSettings();
}

GraftBaseClient::~GraftBaseClient()
{
    delete mQRCodeEncoder;
    delete mAccountManager;
}

void GraftBaseClient::setNetworkType(int networkType)
{
    mAccountManager->setNetworkType(networkType);
    emit networkTypeChanged();
}

int GraftBaseClient::networkType() const
{
    switch (mAccountManager->networkType()) {
    case GraftClientTools::Mainnet:
        return GraftClientTools::Mainnet;
    case GraftClientTools::PublicTestnet:
        return GraftClientTools::PublicTestnet;
    case GraftClientTools::PublicExperimentalTestnet:
        return GraftClientTools::PublicExperimentalTestnet;
    default:
        return -1;
    }
}

bool GraftBaseClient::isAccountExists() const
{
    return !mAccountManager->account().isEmpty();
}

void GraftBaseClient::resetData() const
{
    mAccountManager->clearData();
}

QString GraftBaseClient::getSeed() const
{
    return mAccountManager->seed();
}

QString GraftBaseClient::address() const
{
    return mAccountManager->address();
}

AccountModel *GraftBaseClient::accountModel() const
{
    return mAccountModel;
}

CurrencyModel *GraftBaseClient::currencyModel() const
{
    return mCurrencyModel;
}

QuickExchangeModel *GraftBaseClient::quickExchangeModel() const
{
    return mQuickExchangeModel;
}

void GraftBaseClient::setQRCodeImage(const QImage &image)
{
    if (mImageProvider)
    {
        mImageProvider->setBarcodeImage(scQRCodeImageID, image);
    }
}

void GraftBaseClient::registerTypes(QQmlEngine *engine)
{
    registerImageProvider(engine);
    initAccountModel(engine);
    initCurrencyModel(engine);
    initQuickExchangeModel(engine);
    qmlRegisterUncreatableType<GraftClientTools>("org.graft", 1, 0,
                                                 "GraftClientTools",
                                                 "You cannot create an instance of GraftClientTools type.");
}

QString GraftBaseClient::qrCodeImage() const
{
    return scProviderScheme.arg(scBarcodeImageProviderID).arg(scQRCodeImageID);
}

QString GraftBaseClient::addressQRCodeImage() const
{
    if (mImageProvider && mImageProvider->barcodeImage(scAddressQRCodeImageID).isNull())
    {
        updateAddressQRCode();
    }
    return scProviderScheme.arg(scBarcodeImageProviderID).arg(scAddressQRCodeImageID);
}

QString GraftBaseClient::coinAddressQRCodeImage(const QString &address) const
{
    mImageProvider->setBarcodeImage(scCoinAddressQRCodeImageID, mQRCodeEncoder->encode(address));
    return scProviderScheme.arg(scBarcodeImageProviderID).arg(scCoinAddressQRCodeImageID);
}

void GraftBaseClient::saveAccounts() const
{
    saveModel(scAccountModelDataFile, AccountModelSerializator::serialize(mAccountModel));
}

void GraftBaseClient::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mBalanceTimer && !mAccountManager->account().isEmpty())
    {
        updateBalance();
    }
}

void GraftBaseClient::registerImageProvider(QQmlEngine *engine)
{
    if (!mImageProvider)
    {
        mImageProvider = new BarcodeImageProvider();
        engine->addImageProvider(scBarcodeImageProviderID, mImageProvider);
    }
}

void GraftBaseClient::saveModel(const QString &fileName, const QByteArray &data) const
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!QFileInfo(dataPath).exists())
    {
        QDir().mkpath(dataPath);
    }
    QDir lDir(dataPath);
    QFile lFile(lDir.filePath(fileName));
    if (lFile.open(QFile::WriteOnly))
    {
        lFile.write(data);
        lFile.close();
    }
}

QByteArray GraftBaseClient::loadModel(const QString &fileName) const
{
    QString dataPath = QStandardPaths::locate(QStandardPaths::AppDataLocation,
                                              fileName);
    if (!dataPath.isEmpty())
    {
        QFile lFile(dataPath);
        if (lFile.open(QFile::ReadOnly))
        {
            return lFile.readAll();
        }
    }
    return QByteArray();
}

QUrl GraftBaseClient::getServiceUrl() const
{
    QString finalUrl;
    if (useOwnServiceAddress())
    {
        QString ip(settings("ip").toString());
        QString port(settings("port").toString());
        finalUrl = QString("%1:%2").arg(ip).arg(port);
    }
    else
    {
        QStringList seedNodes = seedSupernodes();
        finalUrl = seedNodes.value(qrand() % seedNodes.count());
    }
    return QUrl(scUrl.arg(finalUrl));
}

void GraftBaseClient::requestAccount(GraftGenericAPI *api, const QString &password)
{
    if (api)
    {
        if (mAccountManager->account().isEmpty())
        {
            connect(api, &GraftGenericAPI::createAccountReceived,
                    this, &GraftBaseClient::receiveAccount, Qt::UniqueConnection);
            mAccountManager->setPassword(password);
            api->createAccount(password);
        }
        else
        {
            api->setAccountData(mAccountManager->account(), mAccountManager->passsword());
        }
    }
}

void GraftBaseClient::requestRestoreAccount(GraftGenericAPI *api, const QString &seed,
                                            const QString &password)
{
    if (api)
    {
        connect(api, &GraftGenericAPI::restoreAccountReceived,
                this, &GraftBaseClient::receiveRestoreAccount, Qt::UniqueConnection);
        mAccountManager->setPassword(password);
        api->restoreAccount(seed, password);
    }
}

void GraftBaseClient::registerBalanceTimer(GraftGenericAPI *api)
{
    if (api)
    {
        connect(api, &GraftGenericAPI::getBalanceReceived, this, &GraftBaseClient::receiveBalance,
                Qt::UniqueConnection);
        mBalanceTimer = startTimer(20000);
    }
}

void GraftBaseClient::receiveAccount(const QByteArray &accountData, const QString &password,
                                     const QString &address, const QString &viewKey,
                                     const QString &seed)
{
    bool isAccountCreated = false;
    if (mAccountManager->passsword() == password && !accountData.isEmpty() && !address.isEmpty())
    {
        mAccountManager->setAccount(accountData);
        mAccountManager->setAddress(address);
        mAccountManager->setViewKey(viewKey);
        mAccountManager->setSeed(seed);
        updateAddressQRCode();
        isAccountCreated = true;
    }
    emit createAccountReceived(isAccountCreated);
}

void GraftBaseClient::receiveRestoreAccount(const QByteArray &accountData, const QString &password,
                                            const QString &address, const QString &viewKey,
                                            const QString &seed)
{
    bool isAccountRestored = false;
    if (mAccountManager->passsword() == password && !accountData.isEmpty() &&!address.isEmpty())
    {
        mAccountManager->setAccount(accountData);
        mAccountManager->setAddress(address);
        mAccountManager->setViewKey(viewKey);
        mAccountManager->setSeed(seed);
        updateAddressQRCode();
        isAccountRestored = true;
    }
    emit restoreAccountReceived(isAccountRestored);
}

void GraftBaseClient::receiveBalance(double balance, double unlockedBalance)
{
    if (balance >= 0 && unlockedBalance >= 0)
    {
        mBalances.insert(GraftClientTools::LockedBalance, balance - unlockedBalance);
        mBalances.insert(GraftClientTools::UnlockedBalance, unlockedBalance);
        mBalances.insert(GraftClientTools::LocalBalance, unlockedBalance);
        emit balanceUpdated();
    }
}

void GraftBaseClient::initAccountModel(QQmlEngine *engine)
{
    if(!mAccountModel)
    {
        mAccountModel = new AccountModel(this);
        engine->rootContext()->setContextProperty(QStringLiteral("AccountModel"), mAccountModel);
        AccountModelSerializator::deserialize(loadModel(scAccountModelDataFile), mAccountModel);
    }
}

void GraftBaseClient::initCurrencyModel(QQmlEngine *engine)
{
    if(!mCurrencyModel)
    {
        mCurrencyModel = new CurrencyModel(this);
        mCurrencyModel->add(QStringLiteral("BITCOIN"), QStringLiteral("BTC"));
        mCurrencyModel->add(QStringLiteral("BITCONNECT COIN"), QStringLiteral("BCC"));
        mCurrencyModel->add(QStringLiteral("DASH"), QStringLiteral("DASH"));
        mCurrencyModel->add(QStringLiteral("ETHER"), QStringLiteral("ETH"));
        mCurrencyModel->add(QStringLiteral("LITECOIN"), QStringLiteral("LTC"));
        mCurrencyModel->add(QStringLiteral("NEW ECONOMY MOVEMENT"), QStringLiteral("NEM"));
        mCurrencyModel->add(QStringLiteral("NEO"), QStringLiteral("NEO"));
        mCurrencyModel->add(QStringLiteral("RIPPLE"), QStringLiteral("XRP"));
        mCurrencyModel->add(QStringLiteral("MONERO"), QStringLiteral("XMR"));
        engine->rootContext()->setContextProperty(QStringLiteral("CoinModel"), mCurrencyModel);
    }
}

void GraftBaseClient::initQuickExchangeModel(QQmlEngine *engine)
{
    if(!mQuickExchangeModel)
    {
        mQuickExchangeModel = new QuickExchangeModel(this);
        mQuickExchangeModel->add(QStringLiteral("US Dollar"), QStringLiteral("USD"),
                                 QString(), true);
        for (CurrencyItem *item : mCurrencyModel->currencies())
        {
            mQuickExchangeModel->add(item->name(), item->code());
        }
        engine->rootContext()->setContextProperty(QStringLiteral("QuickExchangeModel"),
                                                  mQuickExchangeModel);
    }
}

void GraftBaseClient::updateAddressQRCode() const
{
    mImageProvider->setBarcodeImage(scAddressQRCodeImageID, mQRCodeEncoder->encode(address()));
}

void GraftBaseClient::setSettings(const QString &key, const QVariant &value)
{
    mClientSettings->setValue(key, value);
}

bool GraftBaseClient::useOwnServiceAddress() const
{
    return mClientSettings->value(QStringLiteral("useOwnServiceAddress")).toBool();
}

bool GraftBaseClient::resetUrl(const QString &ip, const QString &port)
{
    bool lIsResetUrl = (useOwnServiceAddress() && isValidIp(ip) && !ip.isEmpty());
    if (lIsResetUrl)
    {
        setSettings(QStringLiteral("ip"), ip);
        setSettings(QStringLiteral("port"), port);
    }
    return lIsResetUrl;
}

bool GraftBaseClient::isValidIp(const QString &ip) const
{
    QHostAddress validateIp;
    return validateIp.setAddress(ip);
}

double GraftBaseClient::balance(int type) const
{
    QString rValue = QString::number(mBalances.value(type, 0), 'f', 4);
    return rValue.toDouble();
}

void GraftBaseClient::updateQuickExchange(double cost)
{
    QStringList codes = mQuickExchangeModel->codeList();
    for (int i = 0; i < codes.count(); ++i)
    {
        double course = 1.0;
        if (codes.value(i) != QLatin1String("USD"))
        {
            course = (double)qrand() / RAND_MAX;
        }
        mQuickExchangeModel->updatePrice(codes.value(i), QString::number(course * cost));
    }
}

bool GraftBaseClient::checkPassword(const QString &password) const
{
    return mAccountManager->passsword() == password;
}

void GraftBaseClient::copyWalletNumber(const QString &walletNumber) const
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(walletNumber);
}

QString GraftBaseClient::networkName() const
{
    switch (mAccountManager->networkType())
    {
    case GraftClientTools::Mainnet:
        return MainnetConfiguration::scConfigTitle;
    case GraftClientTools::PublicTestnet:
        return TestnetConfiguration::scConfigTitle;
    case GraftClientTools::PublicExperimentalTestnet:
        return ExperimentalTestnetConfiguration::scConfigTitle;
    default:
        return QString();
    }
}

QString GraftBaseClient::dapiVersion() const
{
    switch (mAccountManager->networkType())
    {
    case GraftClientTools::Mainnet:
        return MainnetConfiguration::scDAPIVersion;
    case GraftClientTools::PublicTestnet:
        return TestnetConfiguration::scDAPIVersion;
    case GraftClientTools::PublicExperimentalTestnet:
        return ExperimentalTestnetConfiguration::scDAPIVersion;
    default:
        return QString();
    }
}

QStringList GraftBaseClient::seedSupernodes() const
{
    switch (mAccountManager->networkType())
    {
    case GraftClientTools::Mainnet:
        return MainnetConfiguration::scSeedSupernodes;
    case GraftClientTools::PublicTestnet:
        return TestnetConfiguration::scSeedSupernodes;
    case GraftClientTools::PublicExperimentalTestnet:
        return ExperimentalTestnetConfiguration::scSeedSupernodes;
    default:
        return QStringList();
    }
}

QVariant GraftBaseClient::settings(const QString &key) const
{
    return mClientSettings->value(key);
}

void GraftBaseClient::saveSettings() const
{
    mClientSettings->sync();
}

void GraftBaseClient::initSettings()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!QFileInfo(dataPath).exists())
    {
        QDir().mkpath(dataPath);
    }
    QDir lDir(dataPath);
    mClientSettings = new QSettings(lDir.filePath(scSettingsDataFile), QSettings::IniFormat, this);
}
