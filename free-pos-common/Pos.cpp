#include "Pos.h"
#include "Ticket.h"
#include "Customer.h"
#include "OrderItem.h"
#include "OrderItemInventoryItem.h"
#include "MenuItemOption.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QMessageBox>
#include <QDateTime>


Pos* Pos::s_instance = nullptr;



Pos* Pos::instance()
{
    if(!s_instance) {
        qDebug() << "Creating singleton...";
        s_instance = new Pos();
    }

    return Pos::s_instance;
}



Pos::Pos(QObject *parent) :
    QObject(parent), m_isHistoryDisabled(false), m_selectedMenu(nullptr), m_selectedRec(nullptr), m_selectedInventory(nullptr), m_selectedWeek(nullptr), m_isDialogMessageShown(false) //, m_weekCurrId(0)
{
}

void Pos::readHistory(QString filename) {

    m_isHistoryDisabled = true;

    QFile file("./data/" + filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);

    while(!in.atEnd()) {
        QString line = in.readLine();
        int delimit = line.indexOf(":");
        QString timestamp = line.mid(0, delimit);
        Q_UNUSED(timestamp);
        QString theRest = line.mid(delimit+1, line.length() - delimit);
        delimit = theRest.indexOf(":");
        QString command = theRest.mid(0, delimit);
        QString payload = theRest.mid(delimit+1, line.length() - delimit);
        QStringList split = SimpleSerializable::deserializeList(payload);

        /*if(command == "OpenRec") {
            qDebug() << "Open new rec" << payload;
            m_selectedRec = Reconciliation::deserialize(payload, this);
        } else if(command == "AddTicket") {
            qDebug() << "AddTicket: " << payload;
            m_selectedRec->addTicket(Ticket::deserialize(payload, this));
        } else if(command == "AddCustomer") {
            qDebug() << "AddCustomer: " << payload;

            Customer *c = Customer::deserialize(payload, this);
            Ticket *ticket = m_selectedRec->getTicket(c->property("ticketId").toInt());
            if(ticket)
                ticket->addCustomer(c);
            else
                qDebug() << "Ticket does not exist.";
        } else if(command == "AddOrderItem") {
            qDebug() << "AddOrderItem: " << payload;
            OrderItem *i = OrderItem::deserialize(payload);
            Ticket *ticket = m_selectedRec->getTicket(i->property("ticketId").toInt());
            if(!ticket) {
                qDebug() << "Ticket does not exist.";
                return;
            }
            Customer *customer = ticket->getCustomer(i->property("customerId").toUInt());
            if(!customer) {
                qDebug() << "Customer does not exist.";
                return;
            }
            customer->addOrderItem(i);
        } else if (command == "UpdateReconciliation") {
            qDebug() << "UpdateReconciliation: " << payload;
            //quint32 recId = split[0].toUInt();
            QString property = split[1];
            QString value = split[2];
            m_selectedRec->setProperty(property.toUtf8().data(), value);
        } else if (command == "UpdateCashDrawer") {
            qDebug() << "UpdateCashDrawer: " << payload;
            //quint32 recId = split[0].toUInt();
            quint32 cashDrawerId = split[0].toUInt();
            QString property = split[1];
            QString value = split[2];
            if(cashDrawerId == 1) {
                m_selectedRec->beginningDrawer()->setProperty(property.toUtf8().data(), value);
            } else if(cashDrawerId == 2) {
                m_selectedRec->endingDrawer()->setProperty(property.toUtf8().data(), value);
            }
        } else if (command == "UpdateTicket") {
            qDebug() << "UpdateTicket: " << payload;            
            quint32 ticketId = split[0].toUInt();
            QString property = split[1];
            QString value = split[2];
            Ticket* ticket = m_selectedRec->getTicket(ticketId);
            ticket->setProperty(property.toUtf8().data(), value);
        } else if (command == "UpdateCustomer") {
            qDebug() << "UpdateCustomer: " << payload;
            quint32 ticketId = split[0].toUInt();
            quint32 customerId = split[1].toUInt();
            QString property = split[2];
            QString value = split[3];
            Customer* customer = m_selectedRec->getTicket(ticketId)->getCustomer(customerId);
            customer->setProperty(property.toUtf8().data(), value);
        } else if (command == "UpdateOrderItem") {
            qDebug() << "UpdateOrderItem: " << payload;
            quint32 ticketId = split[0].toUInt();
            quint32 customerId = split[1].toUInt();
            quint32 orderItemId = split[2].toUInt();
            QString property = split[3];
            QString value = split[4];
            OrderItem* orderItem = m_selectedRec->getTicket(ticketId)->getCustomer(customerId)->getOrderItem(orderItemId);
            if(property == "submittedStamp" && value.trimmed() == "") {
                orderItem->setProperty("submittedStamp", QDateTime());
            } else {
                orderItem->setProperty(property.toUtf8().data(), value);
            }
        } else if (command == "MoveOrderItem") {
            qDebug() << "MoveOrderItem: " << payload;
            quint32 fromTicketId = split[0].toUInt();
            quint32 fromCustomerId = split[1].toUInt();
            quint32 orderItemId = split[2].toUInt();
            quint32 toTicketId = split[3].toUInt();
            quint32 toCustomerId = split[4].toUInt();

            OrderItem* orderItem = m_selectedRec->getTicket(fromTicketId)->getCustomer(fromCustomerId)->getOrderItem(orderItemId);
            m_selectedRec->moveOrderItem(orderItem, toTicketId, toCustomerId);
        } else*/

        if (command == "AddMenu") {
            //qDebug() << "AddMenu: " << payload;
            //quint32 id = split[0].toUInt();
            m_selectedMenu = new Menu(this);
            Pos::instance()->addMenu(m_selectedMenu);
            selectedMenuChanged(m_selectedMenu);
        } else if(command == "AddMenuCategory") {
            //qDebug() << "AddMenuCategory: " << payload;
            quint32 id = split[0].toUInt();
            QString name = split[1];
            m_selectedMenu->addCategory(new MenuCategory(this, id, name));
        } else if(command == "AddMenuItem") {
            //qDebug() << "AddMenuItem: " << payload;
            quint32 id = split[0].toUInt();
            quint32 menuCategoryId = split[1].toUInt();
            QString name = split[2];
            QString type = split[3];
            float price = split[4].toFloat();
            MenuCategory *category = m_selectedMenu->getMenuCategory(menuCategoryId);
            category->addMenuItem(new MenuItem(this, id, menuCategoryId, name, type, price));
        } else if (command == "UpdateMenuCategory") {
            //qDebug() << "UpdateMenuCategory: " << payload;
            quint32 menuCategoryId = split[0].toUInt();
            QString property = split[1];
            QString value = split[2];
            MenuCategory* cat = m_selectedMenu->getMenuCategory(menuCategoryId);
            cat->setProperty(property.toUtf8().data(), value);
        } else if (command == "UpdateMenuItem") {
            //qDebug() << "UpdateMenuItem: " << payload;
            quint32 id = split[0].toUInt();
            quint32 menuCategoryId = split[1].toUInt();
            QString property = split[2];
            QString value = split[3];
            MenuCategory *cat = m_selectedMenu->getMenuCategory(menuCategoryId);
            MenuItem *item = cat->getMenuItem(id);
            item->setProperty(property.toUtf8().data(), value);
        } else if (command == "AddInventory") {
            //qDebug() << "AddInventory: " << payload;
            m_selectedInventory = new Inventory(this);
            selectedInventoryChanged(m_selectedInventory);
        } else if(command == "AddInventoryItem") {
            //qDebug() << "AddInventoryItem: " << payload;
            m_selectedInventory->addInventoryItem(InventoryItem::deserialize(payload, this));
        } else if (command == "UpdateInventoryItem") {
            //qDebug() << "UpdateInventoryItem: " << payload;
            quint32 id = split[0].toUInt();
            QString property = split[1];
            QString value = split[2];
            InventoryItem *item = m_selectedInventory->getInventoryItem(id);
            item->setProperty(property.toUtf8().data(), value);
        } else if (command == "AddMenuItemInventoryItem") {
            //qDebug() << "AddMenuItemInventoryItem: " << payload;
            MenuItemInventoryItem *menuItemInventoryItem = MenuItemInventoryItem::deserialize(payload);
            InventoryItem *inventoryItem = Pos::instance()->selectedInventory()->getInventoryItem(menuItemInventoryItem->property("inventoryItemId").toUInt());
            menuItemInventoryItem->setInventoryItem(inventoryItem);
            MenuCategory* cat = Pos::instance()->selectedMenu()->getMenuCategory(menuItemInventoryItem->property("menuCategoryId").toUInt());
            MenuItem *menuItem = cat->getMenuItem(menuItemInventoryItem->property("menuItemId").toUInt());
            menuItem->addMenuItemInventoryItem(menuItemInventoryItem);
        } else if (command == "UpdateMenuItemInventoryItem") {
            //qDebug() << "UpdateMenuItemInventoryItem: " << payload;
            quint32 id = split[0].toUInt();
            quint32 menuCategoryId = split[1].toUInt();
            quint32 menuItemId = split[2].toUInt();
            QString property = split[3];
            QString value = split[4];
            MenuCategory *cat = m_selectedMenu->getMenuCategory(menuCategoryId);
            MenuItem *menuItem = cat->getMenuItem(menuItemId);
            MenuItemInventoryItem *item = menuItem->getMenuItemInventoryItem(id);
            item->setProperty(property.toUtf8().data(), value);
        } else if (command == "RemoveMenuItemInventoryItem") {
            //qDebug() << "RemoveMenuItemInventoryItem: " << payload;
            quint32 id = split[0].toUInt();
            quint32 menuCategoryId = split[1].toUInt();
            quint32  menuItemId = split[2].toUInt();
            MenuCategory* cat = Pos::instance()->selectedMenu()->getMenuCategory(menuCategoryId);
            MenuItem *menuItem = cat->getMenuItem(menuItemId);
            menuItem->removeMenuItemInventoryItem(id);
        } else if (command == "AddMenuItemOption") {
            //qDebug() << "AddMenuItemOption: " << payload;
            MenuItemOption *menuItemOption = MenuItemOption::deserialize(payload);
            MenuCategory* cat = Pos::instance()->selectedMenu()->getMenuCategory(menuItemOption->menuCategoryId());
            MenuItem *menuItem = cat->getMenuItem(menuItemOption->menuItemId());
            menuItem->addMenuItemOption(menuItemOption);
        } else {
            qDebug() << "Unknown command: " << command << " " << payload;
        }
    }

    // optional, as QFile destructor will already do it:
    file.close();

    m_isHistoryDisabled = false;
}



void Pos::appendToHistory(QString item) {
    if(!m_selectedRec) {
        qDebug() << "No selected rec!";
    } else {
        m_selectedRec->appendToHistory(item);
    }
}

void Pos::appendToMenuHistory(QString item) {
    appendToFile(item, "currMenu.txt");
}

void Pos::appendToInventoryHistory(QString item) {
    appendToFile(item, "currInventory.txt");
}

void Pos::appendToTrackerHistory(QString item) {
    appendToFile(item, "currTracker.txt");
}

void Pos::appendToFixedCostHistory(QString item) {    
    appendToFile(item, "currFixedCost.txt");
}

void Pos::appendToWebServiceLog(QString item) {
    appendToFile(QDateTime::currentDateTime().toString() +": " + item + "\n\n", "webServiceLog.txt");
}

void Pos::appendToFile(QString item, QString filename) {
    if(!m_isHistoryDisabled) {
        QFile file("./data/" + filename);
        file.open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << SimpleSerializable::escapeString(QDateTime::currentDateTime().toString()) << ":" << item << endl;
        file.close();
    }
}


QQmlListProperty<Menu> Pos::menus() {
    return QQmlListProperty<Menu>(this, m_menus);
}

void Pos::addMenu(Menu* menu) {
    m_menus.append(menu);
    menusChanged(menus());
}

QQmlListProperty<Reconciliation> Pos::reconciliations() {
    return QQmlListProperty<Reconciliation>(this, m_reconciliations);
}


void Pos::unselectRec() {
    if(m_selectedRec) {
        m_selectedRec = nullptr;
        selectedRecChanged(m_selectedRec);
    }
}

Reconciliation* Pos::selectedRec() {
    return m_selectedRec;
}


Menu* Pos::selectedMenu() {
    return m_selectedMenu;
}

Inventory* Pos::selectedInventory() {
    return m_selectedInventory;
}


void Pos::addReconciliation(Reconciliation *rec) {
    m_reconciliations.append(rec);
    reconciliationsChanged(reconciliations());
}






WeekTracker* Pos::previousWeek() {
    return getWeek(m_selectedWeek->date().addDays(-7));
}

WeekTracker* Pos::nextWeek() {
    return getWeek(m_selectedWeek->date().addDays(7));
}

QQmlListProperty<WeekTracker> Pos::weeks() {
    return QQmlListProperty<WeekTracker>(this, m_weeks);
}

QList<WeekTracker*> Pos::weeksList() {
    return m_weeks;
}

WeekTracker* Pos::addWeek(QDate startDate) {
    WeekTracker *obj = new WeekTracker(this);
    obj->createDays(startDate);
    //addWeek(obj);
    m_weeks.append(obj);
    weeksChanged(weeks());
    return obj;
}

//void Pos::addWeek(WeekTracker *value) {
//    if(value->id() > m_weekCurrId) m_weekCurrId = value->id();
//    m_weeks.append(value);
//    weeksChanged(weeks());
//}

//WeekTracker* Pos::getWeek(quint32 id) {
//    for(WeekTracker *value : m_weeks) {
//        if(value->property("id").toUInt() == id) {
//            return value;
//        }
//    }
//    return nullptr;
//}

WeekTracker* Pos::getWeek(QDate date) {
    for(WeekTracker *value : m_weeks) {
        if(value->containsDate(date)) {
            return value;
        }
    }

    // Week isn't loaded yet, add it:
    int dayOfWeek = date.dayOfWeek();
    QDate sunday;
    // if not Sunday, then make it Sunday
    if(dayOfWeek < 7) {
        sunday = date.addDays(-1 * dayOfWeek);
    } else {
        sunday = date;
    }

    return addWeek(sunday);
}

//void Pos::removeWeek(quint32 id) {
//    for(int i = 0; i < m_weeks.length(); i++) {
//        if(m_weeks[i]->property("id").toUInt() == id) {
//            //WeekTracker *item = m_weeks[i];
//            m_weeks.removeAt(i);
//            weeksChanged(weeks());
//             return;
//        }
//    }
//}









void Pos::addTestData() {

//    m_selectedMenu = new Menu(this);
//    addMenu(m_selectedMenu);
//    selectedMenuChanged(m_selectedMenu);

//    m_selectedRec = openNewRec();
//    selectedRecChanged(m_selectedRec);

//    Ticket *ticket = m_selectedRec->addTicket("1-1");
////    ticket->addCustomer("Andrew");
////    ticket = m_selectedRec->addTicket("2-2");
////    ticket->addCustomer("Justin");
////    ticket = m_selectedRec->addTicket("Bar");
//    Customer *customer = ticket->addCustomer("Larry");
//    customer->addOrderItem("Steak Bites - Small", "Food", 7.50, 2, "This is a note.");
//    customer->addOrderItem("Steak Bites - Small", "Food", 7.50, 1, "This is a note2.");
//    customer = ticket->addCustomer("Steve");
//    customer->addOrderItem("Steak Bites - Large", "Food", 8.00, 0.5, "This is a note3.");

////    ticket = m_selectedRec->addTicket("Deck");
////    ticket->addCustomer("Tina");
//    m_selectedRec->setSelectedTicket(ticket);

////    for(int i = 1; i <= 50; i++) {
////        ticket = m_selectedRec->addTicket("Bar");
////        customer = ticket->addCustomer("Test Customer " + QString::number(i));
////        for(int j = 0; j < 1; j++) {
////            customer->addOrderItem("Test Food Item", "Food", 7.50, 1, "This is a note.");
////            customer->addOrderItem("Test Alcohol Item", "Alcohol", 2.75, 1, "This is a note.");
////        }
////    }




//#ifdef QT_DEBUG
//    QFile file("../../free-pos/free-pos-server/menu.json");
//#else
//    QFile file("menu.json");
//#endif

//    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        qDebug() << "Could not open menu file" << endl;
//    }

//    QJsonDocument json = QJsonDocument::fromJson(file.readAll());
//    QJsonObject menu = json.object()["categories"].toObject();
//    QJsonObject catObj;
//    QJsonObject itemObj;
//    MenuCategory* newCat;


//    for(QJsonValue cat : menu) {
//        catObj = cat.toObject();
//        //qDebug() << "Adding: " << catObj["name"].toString();
//        newCat = m_selectedMenu->addCategory(catObj["name"].toString());
//        for(QJsonValue item : catObj["menuitems"].toObject()) {
//            itemObj = item.toObject();
//            newCat->addMenuItem(itemObj["name"].toString(), itemObj["class"].toString(), itemObj["price"].toString().toFloat());
//        }
//    }
}

void Pos::getLoyaltyMembers() {
    QNetworkAccessManager *manager =new QNetworkAccessManager(this);
    QUrl url("http://coalyard.azure-mobile.net/tables/loyaltymember");
    //QUrl url("http://localhost:12509/tables/loyaltymember");
    url.setUserName("");
    url.setPassword("XzknuDmYLwaJCTzznRUnRErOIPjJnq59");
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(handleLoyaltyMembers(QNetworkReply*)));
    manager->get(QNetworkRequest(url));
}

void Pos::handleLoyaltyMembers(QNetworkReply * reply) {
    QByteArray response = reply->readAll();
    qDebug() << "Reply: " << response;

    QJsonDocument json = QJsonDocument::fromJson(response);
    QJsonArray members = json.array();

    this->m_loyaltyMembers.clear();
    LoyaltyMember *member;
    foreach (const QJsonValue & value, members) {
        QJsonObject obj = value.toObject();
        qDebug() << "Member: " + obj["id"].toString() + " " + obj["firstName"].toString();
        member = new LoyaltyMember(this,
                obj["id"].toString(),
                obj["firstName"].toString(),
                obj["lastName"].toString(),
                obj["emailAddress"].toString(),
                obj["pointsBalance"].toDouble());
        this->m_loyaltyMembers.append(member);
    }
    loyaltyMembersChanged(loyaltyMembers());

    qDebug() << "Members count: " << m_loyaltyMembers.length();
}

QQmlListProperty<LoyaltyMember> Pos::loyaltyMembers() {
    return QQmlListProperty<LoyaltyMember>(this, m_loyaltyMembers);
}

void Pos::showDialogMessage(QString message) {

    this->setProperty("dialogMessage", message);
    this->setProperty("isDialogMessageShown", false);
    this->setProperty("isDialogMessageShown", true);
}
