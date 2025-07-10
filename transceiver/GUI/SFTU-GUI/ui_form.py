# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'form.ui'
##
## Created by: Qt User Interface Compiler version 6.9.1
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QAction, QBrush, QColor, QConicalGradient,
    QCursor, QFont, QFontDatabase, QGradient,
    QIcon, QImage, QKeySequence, QLinearGradient,
    QPainter, QPalette, QPixmap, QRadialGradient,
    QTransform)
from PySide6.QtWidgets import (QApplication, QComboBox, QGroupBox, QMainWindow,
    QMenu, QMenuBar, QPushButton, QSizePolicy,
    QStatusBar, QTabWidget, QTextEdit, QVBoxLayout,
    QWidget)

class Ui_main(object):
    def setupUi(self, main):
        if not main.objectName():
            main.setObjectName(u"main")
        main.resize(800, 600)
        self.actionSave_config = QAction(main)
        self.actionSave_config.setObjectName(u"actionSave_config")
        self.centralwidget = QWidget(main)
        self.centralwidget.setObjectName(u"centralwidget")
        self.tabWidget = QTabWidget(self.centralwidget)
        self.tabWidget.setObjectName(u"tabWidget")
        self.tabWidget.setGeometry(QRect(0, 9, 801, 481))
        self.Setup = QWidget()
        self.Setup.setObjectName(u"Setup")
        self.groupBox = QGroupBox(self.Setup)
        self.groupBox.setObjectName(u"groupBox")
        self.groupBox.setGeometry(QRect(30, 50, 321, 131))
        self.layoutWidget = QWidget(self.groupBox)
        self.layoutWidget.setObjectName(u"layoutWidget")
        self.layoutWidget.setGeometry(QRect(10, 30, 301, 91))
        self.verticalLayout = QVBoxLayout(self.layoutWidget)
        self.verticalLayout.setObjectName(u"verticalLayout")
        self.verticalLayout.setContentsMargins(0, 0, 0, 0)
        self.comboBox = QComboBox(self.layoutWidget)
        self.comboBox.setObjectName(u"comboBox")

        self.verticalLayout.addWidget(self.comboBox)

        self.connectButton = QPushButton(self.layoutWidget)
        self.connectButton.setObjectName(u"connectButton")
        self.connectButton.setEnabled(False)
        self.connectButton.setCursor(QCursor(Qt.CursorShape.PointingHandCursor))

        self.verticalLayout.addWidget(self.connectButton)

        self.tabWidget.addTab(self.Setup, "")
        self.Console = QWidget()
        self.Console.setObjectName(u"Console")
        self.tabWidget.addTab(self.Console, "")
        self.Outputs = QWidget()
        self.Outputs.setObjectName(u"Outputs")
        self.tabWidget.addTab(self.Outputs, "")
        self.Inputs = QWidget()
        self.Inputs.setObjectName(u"Inputs")
        self.groupBox_2 = QGroupBox(self.Inputs)
        self.groupBox_2.setObjectName(u"groupBox_2")
        self.groupBox_2.setGeometry(QRect(0, 0, 791, 491))
        self.consoleOutput = QTextEdit(self.groupBox_2)
        self.consoleOutput.setObjectName(u"consoleOutput")
        self.consoleOutput.setEnabled(True)
        self.consoleOutput.setGeometry(QRect(10, 30, 771, 381))
        self.consoleOutput.setMouseTracking(False)
        self.consoleOutput.setReadOnly(True)
        self.clearConsole = QPushButton(self.groupBox_2)
        self.clearConsole.setObjectName(u"clearConsole")
        self.clearConsole.setGeometry(QRect(640, 419, 141, 32))
        self.consoleSend = QTextEdit(self.groupBox_2)
        self.consoleSend.setObjectName(u"consoleSend")
        self.consoleSend.setEnabled(True)
        self.consoleSend.setGeometry(QRect(12, 418, 621, 30))
        self.tabWidget.addTab(self.Inputs, "")
        self.thisRSSI = QTextEdit(self.centralwidget)
        self.thisRSSI.setObjectName(u"thisRSSI")
        self.thisRSSI.setGeometry(QRect(10, 500, 110, 26))
        self.thisRSSI.setReadOnly(True)
        self.thatRSSI = QTextEdit(self.centralwidget)
        self.thatRSSI.setObjectName(u"thatRSSI")
        self.thatRSSI.setGeometry(QRect(680, 500, 110, 26))
        self.thatRSSI.setReadOnly(True)
        main.setCentralWidget(self.centralwidget)
        self.statusbar = QStatusBar(main)
        self.statusbar.setObjectName(u"statusbar")
        main.setStatusBar(self.statusbar)
        self.menubar = QMenuBar(main)
        self.menubar.setObjectName(u"menubar")
        self.menubar.setGeometry(QRect(0, 0, 800, 37))
        self.menuFile = QMenu(self.menubar)
        self.menuFile.setObjectName(u"menuFile")
        main.setMenuBar(self.menubar)

        self.menubar.addAction(self.menuFile.menuAction())
        self.menuFile.addAction(self.actionSave_config)

        self.retranslateUi(main)

        self.tabWidget.setCurrentIndex(3)


        QMetaObject.connectSlotsByName(main)
    # setupUi

    def retranslateUi(self, main):
        main.setWindowTitle(QCoreApplication.translate("main", u"main", None))
        self.actionSave_config.setText(QCoreApplication.translate("main", u"Save config", None))
        self.groupBox.setTitle(QCoreApplication.translate("main", u"Connect to Device", None))
        self.connectButton.setText(QCoreApplication.translate("main", u"Connect", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.Setup), QCoreApplication.translate("main", u"Setup", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.Console), QCoreApplication.translate("main", u"Inputs", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.Outputs), QCoreApplication.translate("main", u"Outputs", None))
        self.groupBox_2.setTitle(QCoreApplication.translate("main", u"Console", None))
        self.clearConsole.setText(QCoreApplication.translate("main", u"Clear", None))
        self.consoleSend.setPlaceholderText(QCoreApplication.translate("main", u"Command", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.Inputs), QCoreApplication.translate("main", u"Console", None))
        self.thisRSSI.setHtml(QCoreApplication.translate("main", u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'.AppleSystemUIFont'; font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">RSSI: -00dBm</p></body></html>", None))
        self.thatRSSI.setHtml(QCoreApplication.translate("main", u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'.AppleSystemUIFont'; font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">RSSI: -00dBm</p></body></html>", None))
        self.menuFile.setTitle(QCoreApplication.translate("main", u"File", None))
    # retranslateUi

