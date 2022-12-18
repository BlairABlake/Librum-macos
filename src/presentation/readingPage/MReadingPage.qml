import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import CustomComponents 1.0
import Librum.style 1.0
import Librum.elements 1.0
import Librum.controllers 1.0
import Librum.globals 1.0
import "readingToolbar"
import "readingSearchbar"


Page
{
    id: root
    property bool fullScreen: false
    
    background: Rectangle
    {
        anchors.fill: parent
        color: Style.pagesBackground
    }
    
    
    Shortcut
    {
        sequences: [StandardKey.ZoomIn]
        onActivated: documentView.zoom(1.2)
    }
    
    Shortcut
    {
        sequences: [StandardKey.ZoomOut]
        onActivated: documentView.zoom(0.8)
    }
    
    Shortcut
    {
        sequences: ["Up"]
        onActivated: documentView.flick("up")
    }
    
    Shortcut
    {
        sequences: ["Down"]
        onActivated: documentView.flick("down")
    }
    
    Shortcut
    {
        sequences: [StandardKey.MoveToNextPage, "Right"]
        onActivated: documentView.nextPage()
    }
    
    Shortcut
    {
        sequences: [StandardKey.MoveToPreviousPage, "Left"]
        onActivated: documentView.previousPage()
    }
    
    Shortcut
    {
        sequence: "ESC"
        onActivated: if(root.fullScreen) root.exitFullScreen();
    }
    
    
    DocumentItem
    {
        id: documentItem
        onUrlChanged: currentPage = 0
        
        onOpenedChanged:
        {
            if(opened)
            {
                toolbar.currentPageButton.maxPages = pageCount;
                toolbar.bookTitle = windowTitleForDocument;
            }
        }
        
        Component.onCompleted: 
        {
            documentItem.url = Globals.selectedBook.filePath;
        }
    }
    
    
    Item
    {
        id: toolbarReactivationContainer
        width: parent.width
        height: toolbar.height
        z: 1
        
        HoverHandler
        {
            id: toolbarReactivationArea
            enabled: true
            onHoveredChanged:
            {
                if(hovered)
                    root.exitFullScreen();
            }
        }
    }
    
    
    ColumnLayout
    {
        id: mainLayout
        anchors.fill: parent
        spacing: 0
        
        
        MReadingToolBar
        {
            id: toolbar            
            Layout.fillWidth: true
            
            currentPage: documentView.document.currentPage
            
            onBackButtonClicked:
            {
                // Save current page
                var operationsMap = {};
                operationsMap[BookController.MetaProperty.CurrentPage] = documentItem.currentPage + 1;
                BookController.updateBook(Globals.selectedBook.uuid, operationsMap);
                
                loadPage(homePage, sidebar.homeItem, false);
            }
            
            onChapterButtonClicked:
            {
                if(chapterSidebar.active)
                {
                    chapterSidebar.close();
                    return;
                }
                
                if(bookmarksSidebar.active)
                    bookmarksSidebar.close();
                
                chapterSidebar.open();
            }
            
            onBookMarkButtonClicked:
            {
                if(bookmarksSidebar.active)
                {
                    bookmarksSidebar.close();
                    return;
                }
                
                if(chapterSidebar.active)
                    chapterSidebar.close();
                
                bookmarksSidebar.open();
            }
            
            onCurrentPageButtonClicked:
            {
                currentPageButton.active = !currentPageButton.active;
            }
            
            onFullScreenButtonClicked:
            {
                if(root.fullScreen) root.exitFullScreen();
                else root.enterFullScreen();
            }
            
            onSearchButtonClicked:
            {
                searchbar.visible = !searchbar.visible;
            }
            
            onOptionsPopupVisibileChanged:
            {
                optionsButton.active = !optionsButton.active
            }
            
            
            PropertyAnimation
            {
                id: hideToolbar
                target: toolbar
                property: "opacity"
                to: 0
                duration: 100
                
                onFinished: toolbar.visible = false
            }
            
            PropertyAnimation
            {
                id: showToolbar
                target: toolbar
                property: "opacity"
                to: 1
                duration: 200
                
                onStarted: toolbar.visible = true
            }
        }
        
        SplitView
        {
            Layout.fillHeight: true
            Layout.fillWidth: true
            orientation: Qt.Horizontal
            padding: 0
            spacing: 10
            handle: Rectangle
            {
                implicitWidth: 8
                color: "transparent"
            }
            smooth: true
            
            // Need to combine sidebars into one item, else rezising doesnt work properly
            Item
            {
                id: sidebarItem
                SplitView.preferredWidth: chapterSidebar.visible ? chapterSidebar.lastWidth 
                                                                 : bookmarksSidebar.visible ? bookmarksSidebar.lastWidth : 0
                SplitView.minimumWidth: chapterSidebar.visible || bookmarksSidebar.visible ? 140 : 0
                SplitView.maximumWidth: 480
                
                visible: chapterSidebar.active || bookmarksSidebar.active
                
                
                MChapterSidebar
                {
                    id: chapterSidebar
                    property int lastWidth: 300
                    property bool active: false
                    
                    onVisibleChanged: if(!visible) lastWidth = width
                    
                    anchors.fill: parent
                    visible: false
                    
                    
                    Rectangle
                    {
                        width: 1
                        height: parent.height
                        color: Style.colorLightBorder
                        anchors.right: parent.right
                    }
                    
                    
                    function open()
                    {
                        chapterSidebar.active = true;
                        chapterSidebar.visible = true;
                        toolbar.chapterButton.active = true;
                    }
                    
                    function close()
                    {
                        chapterSidebar.active = false;
                        chapterSidebar.visible = false;
                        toolbar.chapterButton.active = false;
                    }
                }
                
                MBookmarksSidebar
                {
                    id: bookmarksSidebar
                    property int lastWidth: 300
                    property bool active: false
                    
                    onVisibleChanged: if(!visible) lastWidth = width
                    
                    anchors.fill: parent
                    visible: false
                    
                    
                    Rectangle
                    {
                        width: 1
                        height: parent.height
                        color: Style.colorLightBorder
                        anchors.right: parent.right
                    }
                    
                    
                    function open()
                    {
                        bookmarksSidebar.active = true;
                        bookmarksSidebar.visible = true;
                        toolbar.bookmarksButton.active = true;
                    }
                    
                    function close()
                    {
                        bookmarksSidebar.active = false;
                        bookmarksSidebar.visible = false;
                        toolbar.bookmarksButton.active = false;
                    }
                }
            }
            
            
            RowLayout
            {
                id: displayLayout
                SplitView.fillWidth: true
                SplitView.fillHeight: true
                spacing: 0
                clip: true
                
                
                MDocumentView
                {
                    id: documentView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: documentItem.opened
                    document: documentItem
                }
            }
        }
        
        MReadingSearchbar
        {
            id: searchbar
            visible: false
            Layout.fillWidth: true
            
            onVisibleChanged: toolbar.searchButton.active = visible;
        }
    }
    
    
    Component.onCompleted: root.forceActiveFocus()
    
    
    function enterFullScreen()
    {
        fullScreen = true;
        
        if(toolbar.visible)
            hideToolbar.start();
    }
    
    function exitFullScreen()
    {
        fullScreen = false;
        
        if(!toolbar.visible)
            showToolbar.start();
    }
}