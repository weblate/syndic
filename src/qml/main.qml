import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.1
import Qt.labs.platform 1.1 as Platform
import org.kde.kirigami 2.7 as Kirigami

Kirigami.ApplicationWindow {
    id: root
    title: (priv.pageTitle.length>0 ? priv.pageTitle+" - " : "") + Qt.application.name

    pageStack {
        globalToolBar.showNavigationButtons: priv.isFirstPage ? 0 : Kirigami.ApplicationHeaderStyle.ShowBackButton
        defaultColumnWidth: (priv.itemListProportion * root.width) - (globalDrawer.actualWidth / 2)
        interactive: false
        columnView.scrollDuration: animationSuspendTimer.running ? 0 : Kirigami.Units.longDuration
    }

    globalDrawer: Kirigami.GlobalDrawer {
        id: drawer
        property real actualWidth: drawerOpen && !modal ? width : 0

        width: priv.feedListProportion * root.width
        modal: true
        handleVisible: modal && priv.isFirstPage
        Kirigami.Theme.colorSet: Kirigami.Theme.View

        topContent: ColumnLayout {
            spacing: 0
            Layout.fillWidth: true

            FeedList {
                id: feedList
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredHeight: drawer.height
                onCurrentlySelectedFeedChanged:
                    if (currentlySelectedFeed) pushFeed(currentlySelectedFeed)
                onItemClicked: drawer.drawerOpen = !drawer.modal
            }

            Kirigami.Separator {
                Layout.fillWidth: true
                Layout.leftMargin: -drawer.leftPadding
                Layout.rightMargin: -drawer.rightPadding
            }
        }

        actions: [
            Kirigami.Action {
                text: qsTr("Add Content")
                iconName: "list-add"
                onTriggered: {
                    pushUtilityPage("qrc:/qml/AddFeedPage.qml", {pageRow: pageStack})
                }
            },
            Kirigami.Action {
                text: qsTr("Settings")
                iconName: "settings-configure"
                onTriggered: {
                    pushUtilityPage("qrc:/qml/SettingsPage.qml", {globalSettings: globalSettings})
                }
            },
            Kirigami.Action {
                text: qsTr("About %1").arg(Qt.application.name)
                iconName: "help-about"
                onTriggered: {
                    pushUtilityPage("qrc:/qml/AboutPage.qml")
                }
            }
        ]
    }

    Settings {
        // @disable-check M16
        category: "Window"
        property alias width: root.width
        property alias height: root.height
    }

    Settings {
        id: globalSettings
        // @disable-check M16
        category: "Global"
        property bool automaticUpdates: true
        property int updateInterval: 3600
        property bool runInBackground: true
    }

    Binding {
        target: feedContext
        property: "defaultUpdateInterval"
        value: globalSettings.automaticUpdates ? globalSettings.updateInterval : 0
    }

    StateGroup {
        states: [
            State {
                name: "portrait"
                when: width < height
                PropertyChanges {
                    target: priv
                    itemListProportion: 1
                    feedListProportion: 0.23
                }
                PropertyChanges {
                    target: globalDrawer
                    drawerOpen: false
                    modal: true
                }
            },

            State {
                name: "widescreen"
                when: width > (height *1.6)
                PropertyChanges {
                    target: priv
                    itemListProportion: 0.38
                    feedListProportion: 0.15
                }
                PropertyChanges {
                    /* separated to avoid stacking errors */
                    target: globalDrawer
                    drawerOpen: true
                }
                PropertyChanges {
                    target: globalDrawer
                    modal: false
                }
            }
        ]
    }

    QtObject {
        id: priv
        property real itemListProportion:  0.38
        property real feedListProportion: 0.23
        property bool isFirstPage: pageStack.firstVisibleItem === pageStack.items[0]
        property string pageTitle: pageStack.firstVisibleItem ? pageStack.firstVisibleItem.title : ""
    }

    Timer {
        id: animationSuspendTimer
        interval: Kirigami.Units.longDuration
    }

    Platform.SystemTrayIcon {
        id: trayIcon
        visible: root.visible == false
        icon.name: "feed-subscribe"
        menu: Platform.Menu {
            Platform.MenuItem {
                text: qsTr("Quit");
                iconName: "application-exit"
                onTriggered: {
                    Qt.quit()
                }
            }
        }
        onActivated: {
            if (reason === Platform.SystemTrayIcon.Context) {
                menu.open(trayIcon)
            } else {
                root.visible = true
            }
        }
    }

    Connections {
        target: pageStack.currentItem
        ignoreUnknownSignals: true
        onBackRequested: {
            // go back to the first non-visible page (rather than the
            // default back behavior, which just moves the focus
            // one column to the left)
            var index = pageStack.firstVisibleItem.Kirigami.ColumnView.index;
            if (index > 0) {
                pageStack.currentIndex = index-1
                event.accepted = true
            }
        }
        onSuspendAnimations: {
            // emitted by pages to temporarily suspend the page transition animation
            animationSuspendTimer.start()
        }
    }

    onClosing: {
        if (globalSettings.runInBackground) {
            close.accepted = false
            root.visible = false
        } else {
            Qt.quit()
        }
    }

    function pushFeed(feed) {
        pageStack.clear()
        pageStack.push("qrc:/qml/ArticleList/ArticleListPage.qml",
                       {pageRow: pageStack,
                           feed: feed})
    }

    function pushUtilityPage(pageUrl, pageProps) {
        feedList.currentIndex = -1
        feedList.currentlySelectedFeed = null
        pageStack.clear()
        pageStack.push(pageUrl, pageProps)
    }
}
