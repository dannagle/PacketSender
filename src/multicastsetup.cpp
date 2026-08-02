#include "multicastsetup.h"
#include "ui_multicastsetup.h"

#include <QMessageBox>

MulticastSetup::MulticastSetup(PacketNetwork *pNetwork, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MulticastSetup)
{
    this->packetNetwork = pNetwork;
    ui->setupUi(this);

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setWindowTitle(tr("IPv4 Multicast Setup (Experimental)"));

    init();
}

void MulticastSetup::setIP(QString ip)
{
    ui->ipaddressEdit->setText(ip);
    ui->joinButton->setFocus();
}

void MulticastSetup::init()
{
    QList<int> udpPorts = this->packetNetwork->getUDPPortsBound();

    if(udpPorts.isEmpty()) {
        ui->infoLabel->setText(tr("There are no bound UDP ports"));
    } else {
        int joinedPort = udpPorts.first();
        QString infoText = tr("UDP socket bound to ");
        infoText.append(QString::number(joinedPort));
        infoText.append(tr(" will join the multicast group"));
        ui->infoLabel->setText(infoText);
    }

    QStringList mcastStringList = packetNetwork->multicastStringList();
    ui->mcastLW->clear();
    ui->mcastLW->addItems(mcastStringList);

    // remember previous dropdown selection if the dialog is being updated or repoened
    QString previousSelection;
    if (ui->interfaceListDropDown->currentIndex() >= 0) {
        previousSelection = ui->interfaceListDropDown->currentText();
    }

    // populate dropdown with potentially useful candidates
    ui->interfaceListDropDown->clear();

    const auto interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &iface : interfaces) {
        // Skip obviously useless ones
        if (!iface.isValid()) continue;
        if (iface.flags() & QNetworkInterface::IsLoopBack) continue;
        if (!(iface.flags() & QNetworkInterface::IsUp)) continue;
        if (!(iface.flags() & QNetworkInterface::IsRunning)) continue;
        if (!(iface.flags() & QNetworkInterface::CanMulticast)) continue;

        // Build a readable name
        QString name = iface.humanReadableName();
        if (name.isEmpty()) name = iface.name();

        // Optionally show the first IPv4 address
        QString ipStr;
        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol && !entry.ip().isLoopback()) {
                ipStr = entry.ip().toString();
                break;
            }
        }

        // Optional: skip interfaces with no IPv4 (removes most utun/anpi/awdl noise)
        if (ipStr.isEmpty()) continue;

        QString display = name;
        if (!ipStr.isEmpty())
            display += QString(" (%1)").arg(ipStr);

        // Store the whole QNetworkInterface as user data
        ui->interfaceListDropDown->addItem(display, QVariant::fromValue(iface));
    }

    int idx = ui->interfaceListDropDown->findText(previousSelection);
    if (idx >= 0) {
        ui->interfaceListDropDown->setCurrentIndex(idx);          // restore previous
    } else if (ui->interfaceListDropDown->count() > 0) {
        ui->interfaceListDropDown->setCurrentIndex(0);            // fallback to first
    }
}

MulticastSetup::~MulticastSetup()
{
    delete ui;
}

void MulticastSetup::on_joinButton_clicked()
{

    QString ip = ui->ipaddressEdit->text().trimmed();

    if (!PacketNetwork::isMulticast(ip)) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Not Multicast."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("IP must be an IPv4 multicast address.\n(224.0.0.0 to 239.255.255.255)"));
        msgBox.exec();
        ui->ipaddressEdit->setFocus();
        ui->ipaddressEdit->selectAll();
        return;
    }


    if(!packetNetwork->IPv4Enabled()) {

        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("IPv4-only."));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Packet Sender supports multicast when bound to IPv4. \nTurn off IPv6 and switch to IPv4 mode?"));

        int yesno = msgBox.exec();
        if (yesno == QMessageBox::No) {
            return;
        }
        packetNetwork->setIPmode(4);
        packetNetwork->kill();
        packetNetwork->init();

    }

    QDEBUGVAR(packetNetwork->multicastStringList());
    init();
}

void MulticastSetup::on_leaveButton_clicked()
{

    QDEBUG();
    packetNetwork->leaveMulticast();
    init();
}

void MulticastSetup::on_leaveSelectedGroupButton_clicked()
{
    QListWidgetItem *item = ui->mcastLW->currentItem();
    if (!item) return;

    // The text we put in the list looks like:  "229.148.44.10  on  en0"
    const QString text = item->text();
    const QStringList parts = text.split("  on  ", Qt::SkipEmptyParts);

    if (parts.size() != 2) {
        QDEBUG() << "Could not parse list item:" << text;
        return;
    }

    const QString address = parts[0].trimmed();
    const QString ifaceName = parts[1].trimmed();

    // Find the matching membership
    bool left = false;

    left = packetNetwork->leaveMulticast(address, ifaceName);

    if (!left) {
        QMessageBox::warning(this, tr("Leave failed"),
                             tr("Could not leave the selected group."));
    }

    init();   // refresh the list
}
