#include "Customer.h"

#include <QMetaProperty>
#include <QDebug>
#include "Pos.h"


Customer::Customer(QObject *parent, quint32 id, quint32 ticketId, QString name) :
    SimpleSerializable(parent), m_id(id), m_ticketId(ticketId), m_name(name), m_currentOrderItemId(0)
{
}


QStringList Customer::updatePrefix() {
    return QStringList() << "UpdateCustomer" << QString::number(m_ticketId) << QString::number(m_id);
}


void Customer::setName(QString name) {
    QString normalized = name.toUpper().trimmed();
    if(m_name != normalized) {
        m_name = normalized;
        logPropertyChanged(m_name, "name");
        nameChanged(m_name);
    }
}


float Customer::foodTotal() {
    float sum = 0;
    QString type;
    for(OrderItem *o : m_orderItems) {
        type = o->property("type").toString();
        if(type != "Alcohol") {
            sum += o->subTotal();
        }
    }
    return sum;
}

float Customer::taxTotal() {
    return foodTotal() * 0.09;
}

float Customer::barTotal() {
    float sum = 0;
    QString type;
    for(OrderItem *o : m_orderItems) {
        type = o->property("type").toString();
        if(type == "Alcohol") {
            sum += o->subTotal();
        }
    }
    return sum;
}

float Customer::total() {
    return foodTotal() + taxTotal() + barTotal();
}

void Customer::fireTotalsChanged() {
    foodTotalChanged(foodTotal());
    taxTotalChanged(taxTotal());
    totalChanged(total());
}

OrderItem* Customer::addOrderItem(QString name, QString type, float price, float quantity, QString note) {
    OrderItem* orderItem = new OrderItem(this, ++m_currentOrderItemId, m_ticketId, m_id, name, type, QDateTime::currentDateTime(), price, quantity, note, false);
    addOrderItem(orderItem);
    if(type == "Alcohol" && !orderItem->isSubmitted()) {
        orderItem->setSubmittedStamp(orderItem->property("createdStamp").toDateTime());
    }
    return orderItem;
}

void Customer::addOrderItem(OrderItem *orderItem, bool isMoved) {
    if(orderItem->property("id").toUInt() > m_currentOrderItemId) m_currentOrderItemId = orderItem->property("id").toUInt();
    connect(orderItem, SIGNAL(subTotalChanged(float)),
            this, SLOT(fireTotalsChanged()));
    connect(orderItem, SIGNAL(taxChanged(float)),
            this, SLOT(fireTotalsChanged()));
    connect(orderItem, SIGNAL(totalChanged(float)),
            this, SLOT(fireTotalsChanged()));
    m_orderItems.append(orderItem);
    if(!isMoved) {
        Pos::instance()->appendToHistory("AddOrderItem:" + orderItem->serialize());
    }
    orderItemsChanged(orderItems());
    fireTotalsChanged();
}

QQmlListProperty<OrderItem> Customer::orderItems() {
    return QQmlListProperty<OrderItem>(this, m_orderItems);
}

OrderItem* Customer::getOrderItem(quint32 id) {
    for(OrderItem* item : m_orderItems) {
        if(item->property("id").toUInt() == id){
            return item;
        }
    }
    return nullptr;
}

QList<OrderItem*> Customer::orderItemsList() {
    return m_orderItems;
}

void Customer::removeOrderItem(OrderItem *orderItem) {
    for(int i = 0; i < m_orderItems.length(); i++) {
        if(m_orderItems[i] == orderItem) {
            m_orderItems.removeAt(i);
            orderItemsChanged(orderItems());
            fireTotalsChanged();
            return;
        }
    }
}

QString Customer::serialize() const {
    return QString::number(m_id) + ":" + QString::number(m_ticketId) + ":" + m_name;
}

Customer* Customer::deserialize(QString serialized, QObject *parent)
{
    QStringList split = serialized.split(":");

    quint32 id = split[0].toInt();
    quint32 ticketId = split[1].toInt();
    QString name = split[2];

    Customer *obj = new Customer(parent, id, ticketId, name);
    return obj;
}




