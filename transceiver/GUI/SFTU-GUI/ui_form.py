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
        self.tabWidget.setGeometry(QRect(0, 9, 801, 441))
        self.Setup = QWidget()
        self.Setup.setObjectName(u"Setup")
        self.groupBox = QGroupBox(self.Setup)
        self.groupBox.setObjectName(u"groupBox")
        self.groupBox.setGeometry(QRect(10, 10, 321, 131))
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

        self.groupBox_3 = QGroupBox(self.Setup)
        self.groupBox_3.setObjectName(u"groupBox_3")
        self.groupBox_3.setGeometry(QRect(560, 10, 231, 361))
        self.frequencyGroup = QGroupBox(self.groupBox_3)
        self.frequencyGroup.setObjectName(u"frequencyGroup")
        self.frequencyGroup.setGeometry(QRect(10, 30, 210, 70))
        self.updateFreqButton = QPushButton(self.frequencyGroup)
        self.updateFreqButton.setObjectName(u"updateFreqButton")
        self.updateFreqButton.setEnabled(False)
        self.updateFreqButton.setGeometry(QRect(128, 24, 71, 41))
        self.freqField = QTextEdit(self.frequencyGroup)
        self.freqField.setObjectName(u"freqField")
        self.freqField.setGeometry(QRect(10, 29, 111, 31))
        self.gainGroup = QGroupBox(self.groupBox_3)
        self.gainGroup.setObjectName(u"gainGroup")
        self.gainGroup.setGeometry(QRect(10, 110, 210, 70))
        self.updateGainButton = QPushButton(self.gainGroup)
        self.updateGainButton.setObjectName(u"updateGainButton")
        self.updateGainButton.setEnabled(False)
        self.updateGainButton.setGeometry(QRect(128, 24, 71, 41))
        self.gainField = QTextEdit(self.gainGroup)
        self.gainField.setObjectName(u"gainField")
        self.gainField.setGeometry(QRect(10, 29, 111, 31))
        self.spreadingGroup = QGroupBox(self.groupBox_3)
        self.spreadingGroup.setObjectName(u"spreadingGroup")
        self.spreadingGroup.setGeometry(QRect(10, 190, 210, 70))
        self.updateSpreadButton = QPushButton(self.spreadingGroup)
        self.updateSpreadButton.setObjectName(u"updateSpreadButton")
        self.updateSpreadButton.setEnabled(False)
        self.updateSpreadButton.setGeometry(QRect(128, 24, 71, 41))
        self.spreadingField = QTextEdit(self.spreadingGroup)
        self.spreadingField.setObjectName(u"spreadingField")
        self.spreadingField.setGeometry(QRect(10, 29, 111, 31))
        self.bandwidthGroup = QGroupBox(self.groupBox_3)
        self.bandwidthGroup.setObjectName(u"bandwidthGroup")
        self.bandwidthGroup.setGeometry(QRect(10, 270, 210, 70))
        self.updateBandwidthButton = QPushButton(self.bandwidthGroup)
        self.updateBandwidthButton.setObjectName(u"updateBandwidthButton")
        self.updateBandwidthButton.setEnabled(False)
        self.updateBandwidthButton.setGeometry(QRect(128, 24, 71, 41))
        self.bandwidthField = QTextEdit(self.bandwidthGroup)
        self.bandwidthField.setObjectName(u"bandwidthField")
        self.bandwidthField.setGeometry(QRect(10, 29, 111, 31))
        self.tabWidget.addTab(self.Setup, "")
        self.Console = QWidget()
        self.Console.setObjectName(u"Console")
        self.tareGroup = QGroupBox(self.Console)
        self.tareGroup.setObjectName(u"tareGroup")
        self.tareGroup.setGeometry(QRect(10, 20, 91, 70))
        self.updateTare = QPushButton(self.tareGroup)
        self.updateTare.setObjectName(u"updateTare")
        self.updateTare.setGeometry(QRect(10, 24, 71, 41))
        self.calibrateGroup = QGroupBox(self.Console)
        self.calibrateGroup.setObjectName(u"calibrateGroup")
        self.calibrateGroup.setGeometry(QRect(110, 20, 210, 70))
        self.updateCalibrate = QPushButton(self.calibrateGroup)
        self.updateCalibrate.setObjectName(u"updateCalibrate")
        self.updateCalibrate.setGeometry(QRect(128, 24, 71, 41))
        self.calibrateField = QTextEdit(self.calibrateGroup)
        self.calibrateField.setObjectName(u"calibrateField")
        self.calibrateField.setGeometry(QRect(10, 29, 111, 31))
        self.tabWidget.addTab(self.Console, "")
        self.Outputs = QWidget()
        self.Outputs.setObjectName(u"Outputs")
        self.groupBox_4 = QGroupBox(self.Outputs)
        self.groupBox_4.setObjectName(u"groupBox_4")
        self.groupBox_4.setGeometry(QRect(100, 50, 590, 300))
        self.outButton_4 = QPushButton(self.groupBox_4)
        self.outButton_4.setObjectName(u"outButton_4")
        self.outButton_4.setGeometry(QRect(440, 50, 100, 100))
        self.outButton_1 = QPushButton(self.groupBox_4)
        self.outButton_1.setObjectName(u"outButton_1")
        self.outButton_1.setGeometry(QRect(50, 50, 100, 100))
        self.outButton_6 = QPushButton(self.groupBox_4)
        self.outButton_6.setObjectName(u"outButton_6")
        self.outButton_6.setGeometry(QRect(180, 170, 100, 100))
        self.outButton_3 = QPushButton(self.groupBox_4)
        self.outButton_3.setObjectName(u"outButton_3")
        self.outButton_3.setGeometry(QRect(310, 50, 100, 100))
        self.outButton_5 = QPushButton(self.groupBox_4)
        self.outButton_5.setObjectName(u"outButton_5")
        self.outButton_5.setGeometry(QRect(50, 170, 100, 100))
        self.outButton_7 = QPushButton(self.groupBox_4)
        self.outButton_7.setObjectName(u"outButton_7")
        self.outButton_7.setGeometry(QRect(310, 170, 100, 100))
        self.outButton_2 = QPushButton(self.groupBox_4)
        self.outButton_2.setObjectName(u"outButton_2")
        self.outButton_2.setGeometry(QRect(180, 50, 100, 100))
        self.outButton_8 = QPushButton(self.groupBox_4)
        self.outButton_8.setObjectName(u"outButton_8")
        self.outButton_8.setGeometry(QRect(440, 170, 100, 100))
        self.tabWidget.addTab(self.Outputs, "")
        self.Inputs = QWidget()
        self.Inputs.setObjectName(u"Inputs")
        self.groupBox_2 = QGroupBox(self.Inputs)
        self.groupBox_2.setObjectName(u"groupBox_2")
        self.groupBox_2.setGeometry(QRect(0, 0, 791, 411))
        self.consoleOutput = QTextEdit(self.groupBox_2)
        self.consoleOutput.setObjectName(u"consoleOutput")
        self.consoleOutput.setEnabled(True)
        self.consoleOutput.setGeometry(QRect(10, 30, 771, 331))
        self.consoleOutput.setMouseTracking(False)
        self.consoleOutput.setReadOnly(True)
        self.clearConsole = QPushButton(self.groupBox_2)
        self.clearConsole.setObjectName(u"clearConsole")
        self.clearConsole.setGeometry(QRect(640, 374, 141, 32))
        self.consoleSend = QTextEdit(self.groupBox_2)
        self.consoleSend.setObjectName(u"consoleSend")
        self.consoleSend.setEnabled(True)
        self.consoleSend.setGeometry(QRect(12, 373, 621, 30))
        self.tabWidget.addTab(self.Inputs, "")
        self.thisRSSI = QTextEdit(self.centralwidget)
        self.thisRSSI.setObjectName(u"thisRSSI")
        self.thisRSSI.setGeometry(QRect(4, 455, 110, 78))
        self.thisRSSI.setAutoFormatting(QTextEdit.AutoFormattingFlag.AutoNone)
        self.thisRSSI.setUndoRedoEnabled(False)
        self.thisRSSI.setLineWrapMode(QTextEdit.LineWrapMode.NoWrap)
        self.thisRSSI.setReadOnly(True)
        self.thatRSSI = QTextEdit(self.centralwidget)
        self.thatRSSI.setObjectName(u"thatRSSI")
        self.thatRSSI.setGeometry(QRect(118, 455, 110, 78))
        self.thatRSSI.setUndoRedoEnabled(False)
        self.thatRSSI.setLineWrapMode(QTextEdit.LineWrapMode.NoWrap)
        self.thatRSSI.setReadOnly(True)
        self.system_msgs = QTextEdit(self.centralwidget)
        self.system_msgs.setObjectName(u"system_msgs")
        self.system_msgs.setGeometry(QRect(310, 500, 480, 27))
        self.system_msgs.setUndoRedoEnabled(False)
        self.system_msgs.setLineWrapMode(QTextEdit.LineWrapMode.NoWrap)
        self.system_msgs.setReadOnly(True)
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

        self.tabWidget.setCurrentIndex(0)  # Open Setup tab by default


        QMetaObject.connectSlotsByName(main)
    # setupUi

    def retranslateUi(self, main):
        main.setWindowTitle(QCoreApplication.translate("main", u"main", None))
        self.actionSave_config.setText(QCoreApplication.translate("main", u"Save config", None))
        self.groupBox.setTitle(QCoreApplication.translate("main", u"Connect to Device", None))
        self.connectButton.setText(QCoreApplication.translate("main", u"Connect", None))
        self.groupBox_3.setTitle(QCoreApplication.translate("main", u"LoRa Config", None))
        self.frequencyGroup.setTitle(QCoreApplication.translate("main", u"Frequency (Mhz)", None))
        self.updateFreqButton.setText(QCoreApplication.translate("main", u"Update", None))
        self.freqField.setHtml(QCoreApplication.translate("main", u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'.AppleSystemUIFont'; font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">915.0</p></body></html>", None))
        self.gainGroup.setTitle(QCoreApplication.translate("main", u"Gain (dBm)", None))
        self.updateGainButton.setText(QCoreApplication.translate("main", u"Update", None))
        self.gainField.setHtml(QCoreApplication.translate("main", u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'.AppleSystemUIFont'; font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">20</p></body></html>", None))
        self.spreadingGroup.setTitle(QCoreApplication.translate("main", u"Spreading Factor (kHz)", None))
        self.updateSpreadButton.setText(QCoreApplication.translate("main", u"Update", None))
        self.spreadingField.setHtml(QCoreApplication.translate("main", u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'.AppleSystemUIFont'; font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">20</p></body></html>", None))
        self.bandwidthGroup.setTitle(QCoreApplication.translate("main", u"Bandwidth (kHz)", None))
        self.updateBandwidthButton.setText(QCoreApplication.translate("main", u"Update", None))
        self.bandwidthField.setHtml(QCoreApplication.translate("main", u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'.AppleSystemUIFont'; font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">20</p></body></html>", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.Setup), QCoreApplication.translate("main", u"Setup", None))
        self.tareGroup.setTitle(QCoreApplication.translate("main", u"Tare (Zero)", None))
        self.updateTare.setText(QCoreApplication.translate("main", u"Update", None))
        self.calibrateGroup.setTitle(QCoreApplication.translate("main", u"Calibrate (kg)", None))
        self.updateCalibrate.setText(QCoreApplication.translate("main", u"Update", None))
        self.calibrateField.setHtml(QCoreApplication.translate("main", u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'.AppleSystemUIFont'; font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">1.0</p></body></html>", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.Console), QCoreApplication.translate("main", u"Inputs", None))
        self.groupBox_4.setTitle(QCoreApplication.translate("main", u"Output Control", None))
        self.outButton_4.setText(QCoreApplication.translate("main", u"OUT 4", None))
        self.outButton_1.setText(QCoreApplication.translate("main", u"OUT 1", None))
        self.outButton_6.setText(QCoreApplication.translate("main", u"OUT 6", None))
        self.outButton_3.setText(QCoreApplication.translate("main", u"OUT 3", None))
        self.outButton_5.setText(QCoreApplication.translate("main", u"OUT 5", None))
        self.outButton_7.setText(QCoreApplication.translate("main", u"OUT 7", None))
        self.outButton_2.setText(QCoreApplication.translate("main", u"OUT 2", None))
        self.outButton_8.setText(QCoreApplication.translate("main", u"OUT 8", None))
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
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:11pt;\">ID: None</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:11pt;\">RSSI: -0.00dBm</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:11pt;\""
                        ">Batt: 0.00V</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:11pt;\">Mode: None</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:11pt;\">Stat: Error</span></p></body></html>", None))
        self.thatRSSI.setHtml(QCoreApplication.translate("main", u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'.AppleSystemUIFont'; font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:11pt;\">ID: None</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:11pt;\">RSSI: -0.00dBm</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:11pt;\""
                        ">Batt: 0.00V</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:11pt;\">Mode: None</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:11pt;\">Stat: Error</span></p></body></html>", None))
        self.system_msgs.setHtml(QCoreApplication.translate("main", u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'.AppleSystemUIFont'; font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">System messages</p></body></html>", None))
        self.menuFile.setTitle(QCoreApplication.translate("main", u"File", None))
    # retranslateUi

