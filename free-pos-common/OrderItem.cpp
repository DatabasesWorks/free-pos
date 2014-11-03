#include <QMetaProperty>
#include <QDebug>
#include "OrderItem.h"
#include "Pos.h"

OrderItem::OrderItem(QObject *parent, quint32 id, quint32 ticketId, quint32 customerId, QString name, QString type, QDateTime createdStamp, float price, float quantity, QString note, bool deleted, QDateTime submittedStamp) :
    SimpleSerializable(parent), m_id(id), m_ticketId(ticketId), m_customerId(customerId), m_name(name), m_type(type), m_createdStamp(createdStamp), m_price(price), m_quantity(quantity), m_note(note), m_deleted(deleted), m_submittedStamp(submittedStamp)
{
    connect(this, SIGNAL(quantityChanged(float)),
            this, SLOT(fireTotalsChanged()));
    connect(this, SIGNAL(priceChanged(float)),
            this, SLOT(fireTotalsChanged()));
    connect(this, SIGNAL(deletedChanged(bool)),
            this, SLOT(fireTotalsChanged()));
}

QStringList OrderItem::updatePrefix() {
    return QStringList() << "UpdateOrderItem" << QString::number(m_ticketId) << QString::number(m_customerId) << QString::number(m_id);
}

void OrderItem::setTicketId(quint32 ticketId) {
    if(m_ticketId != ticketId) {
        m_ticketId = ticketId;
        logPropertyChanged(m_ticketId, "ticketId");
        ticketIdChanged(m_ticketId);
    }
}

void OrderItem::setCustomerId(quint32 customerId) {
    if(m_customerId != customerId) {
        m_customerId = customerId;
        logPropertyChanged(m_customerId, "customerId");
        customerIdChanged(m_customerId);
    }
}

void OrderItem::setCreatedStamp(QDateTime createdStamp) {
    if(m_createdStamp != createdStamp){
        m_createdStamp = createdStamp;
        logPropertyChanged(m_createdStamp, "createdStamp");
        createdStampChanged(m_createdStamp);
    }
}

void OrderItem::setQuantity(float quantity) {
    if(m_quantity != quantity) {
        m_quantity = quantity;
        logPropertyChanged(m_quantity, "quantity");
        quantityChanged(m_quantity);
    }
}

void OrderItem::setPrice(float price) {
    if(m_price != price) {
        m_price = price;
        logPropertyChanged(m_price, "price");
        priceChanged(m_price);
    }
}

void OrderItem::setNote(QString note) {
    if(m_note != note) {
        m_note = note;
        logPropertyChanged(m_note, "note");
        noteChanged(m_note);
    }
}

void OrderItem::setDeleted(bool deleted) {
    if(m_deleted != deleted) {
        m_deleted = deleted;
        logPropertyChanged(m_deleted, "deleted");
        deletedChanged(m_deleted);
    }
}

void OrderItem::setSubmittedStamp(QDateTime submittedStamp) {
    if(m_submittedStamp != submittedStamp){
        m_submittedStamp = submittedStamp;
        logPropertyChanged(m_submittedStamp, "submittedStamp");
        submittedStampChanged(m_submittedStamp);
        isSubmittedChanged(isSubmitted());
    }
}

bool OrderItem::isSubmitted() {
    return m_submittedStamp.isValid();
}

void OrderItem::cycleSubmittedStamp() {
    if(m_submittedStamp.isValid()) {
        setSubmittedStamp(QDateTime());
    } else {
        setSubmittedStamp(QDateTime::currentDateTime());
    }
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
    QStringList vals;
    vals << QString::number(m_id) << QString::number(m_ticketId) << QString::number(m_customerId) << m_name << m_type << m_createdStamp.toString() << QString::number(m_price) << QString::number(m_quantity) << m_note << QString::number(m_deleted) << m_submittedStamp.toString();
    return serializeList(vals);
}

OrderItem* OrderItem::deserialize(QString serialized, QObject *parent)
{
    QStringList split = deserializeList(serialized);

    quint32 id = split[0].toInt();
    quint32 ticketId = split[1].toInt();
    quint32 customerId = split[2].toInt();
    QString name = split[3];
    QString type = split[4];
    QDateTime createdStamp = QDateTime::fromString(split[5]);
    float price = split[6].toFloat();
    float quantity = split[7].toFloat();
    QString note = split[8];
    bool deleted = split[9].toInt();

    QDateTime submittedStamp;
    if(split.length() > 10) {
        submittedStamp = QDateTime::fromString(split[10]);
    }

    OrderItem *obj = new OrderItem(parent, id, ticketId, customerId, name, type, createdStamp, price, quantity, note, deleted, submittedStamp);
    return obj;
}




