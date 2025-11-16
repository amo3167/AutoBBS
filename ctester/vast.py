from PyQt4 import QtGui
from ui.mainwindow import MainWindow
import sys

if __name__ == "__main__":
    app = QtGui.QApplication(sys.argv)
    ui = MainWindow()
    ui.show()
    sys.exit(app.exec_())

