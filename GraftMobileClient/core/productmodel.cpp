#include "productmodel.h"
#include "productitem.h"

ProductModel::ProductModel(QObject *parent) : QAbstractListModel(parent)
{}

ProductModel::~ProductModel()
{
    qDeleteAll(mProducts);
}

QVariant ProductModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= mProducts.count())
    {
        return QVariant();
    }

    ProductItem *productItem = mProducts[index.row()];

    switch (role) {
    case TitleRole:
        return productItem->name();
    case CostRole:
        return productItem->cost();
    case ImageRole:
        return productItem->imagePath();
    case SelectedRole:
        return productItem->isSelected();
    case CurrencyRole:
        if(productItem->currency() == QLatin1String("USD"))
        {
            return "$";
        }
        else if (productItem->currency() == QLatin1String("GRAFT"))
        {
            return "g";
        }
        else
        {
            return "$";
        }
    default:
        return QVariant();
    }
}

bool ProductModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && value.isValid() && data(index, role) != value)
    {
        switch (role)
        {
        case TitleRole:
            mProducts[index.row()]->setName(value.toString());
            break;
        case CostRole:
            mProducts[index.row()]->setCost(value.toDouble());
            break;
        case ImageRole:
            mProducts[index.row()]->setImagePath(value.toString());
            break;
        case SelectedRole:
            mProducts[index.row()]->setSelected(value.toBool());
            break;
        case CurrencyRole:
            mProducts[index.row()]->setCurrency(value.toString());
            break;
        default:
            break;
        }
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

int ProductModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mProducts.count();
}

QVector<ProductItem *> ProductModel::products() const
{
    return mProducts;
}

void ProductModel::changeSelection(int index)
{
    ProductItem* lProduct = mProducts.value(index);
    if (lProduct)
    {
        lProduct->changeSelection();
        QVector<int> changeRole;
        changeRole.append(SelectedRole);
        QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex, changeRole);
    }
}

void ProductModel::add(const QString &imagePath, const QString &name, double cost,
                       const QString &currency)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mProducts << (new ProductItem(imagePath, name, cost, currency));
    endInsertRows();
}

QHash<int, QByteArray> ProductModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TitleRole] = "name";
    roles[CostRole] = "cost";
    roles[ImageRole] = "imagePath";
    roles[SelectedRole] = "selected";
    roles[CurrencyRole] = "currency";
    return roles;
}