#include <QMetaProperty>
#include <QDebug>
#include "OrderItem.h"

OrderItem::OrderItem(QObject *parent, quint32 id, QString name, QString type, float price, float quantity, QString note, bool deleted) :
    QObject(parent), m_id(id), m_name(name), m_type(type), m_price(price), m_quantity(quantity), m_note(note), m_deleted(deleted)
{
    connect(this, SIGNAL(quantityChanged(float)),
            this, SLOT(fireTotalsChanged()));
    connect(this, SIGNAL(priceChanged(float)),
            this, SLOT(fireTotalsChanged()));
    connect(this, SIGNAL(deletedChanged(bool)),
            this, SLOT(fireTotalsChanged()));
}

void OrderItem::fireTotalsChanged() {
    subTotalChanged(subTotal());
    taxChanged(tax());
    totalChanged(total());
}

float OrderItem::subTotal() {
    if(m_deleted)
        return 0;
    else
        return (m_quantity * m_price);
}

float OrderItem::tax() {
    if(m_type == "Alcohol") {
        return 0;
    } else {
        return subTotal() * 0.09;
    }
}

float OrderItem::total() {
    return subTotal() + tax();
}


QString OrderItem::serialize() const {
    //m_name.replace(":", "");
    return QString::number(m_id) + ":" + m_name + ":" + m_type + ":" + QString::number(m_price) + ":" + QString::number(m_quantity) + ":" + m_note + ":" + QString::number(m_deleted);
}

OrderItem* OrderItem::deserialize(QString serialized, QObject *parent)
{
    QStringList split = serialized.split(":");

    quint32 id = split[0].toInt();
    QString name = split[1];
    QString type = split[2];
    float price = split[3].toFloat();
    float quantity = split[4].toFloat();
    QString note = split[5];
    bool deleted = split[6].toInt();

    OrderItem *obj = new OrderItem(parent, id, name, type, price, quantity, note, deleted);
    qDebug() << "    deserialized: " << obj->serialize();
    return obj;
}

QTextStream& operator<<(QTextStream& stream, const OrderItem& obj) {
    stream << obj.serialize() << endl;
    return stream;
}
QTextStream& operator>>(QTextStream& stream, OrderItem& obj) {

    QString line = stream.readAll();
    if(line.length() <= 1){
        qDebug() << "Empty line.";
        return stream;
    }
    OrderItem* obj2 = OrderItem::deserialize(line);
    obj.m_id = obj2->m_id;
    obj.m_name = obj2->m_name;
    obj.m_type = obj2->m_type;
    obj.m_price = obj2->m_price;
    obj.m_quantity = obj2->m_quantity;
    obj.m_note = obj2->m_note;
    obj.m_deleted = obj2->m_deleted;

    return stream;
}



