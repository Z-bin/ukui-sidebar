#! [0]
DEFINES += PLATFORM_TYPE
TEMPLATE    =   subdirs
SUBDIRS	    =   ./src/ukui-sidebar-core.pro \
                ./src/plugins/notification_plugin/notification_plugin.pro \
                ./src/plugins/clipboardPlugin/clipboardPlugin.pro \
                ./src/plugins/Clock_deamon/Clock.pro \
                ./src/plugins/ukui_notebook/ukui_notebook.pro

greaterThan(QT_PATCH_VERSION, 5.7): SUBDIRS     +=  ./src/plugins/feedback/feedback.pro

