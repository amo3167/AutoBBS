
from subprocess import check_output
import platform

def installIfNeeded(moduleName, nameOnPip=None, notes="", log=print):
    """ Installs a Python library using pip, if it isn't already installed. """
    from pkgutil import iter_modules
    system = platform.system()

    # Check if the module is installed
    if moduleName not in [tuple_[1] for tuple_ in iter_modules()]:
        log("Installing " + moduleName + notes + " Library for Python")
        if moduleName == 'numpy' and system == "Windows":
            import requests
            user = input("Please enter your asirikuy username")
            password = input("Please enter your asirikuy password")
            data = {'username': user, 'password': password}
            url = 'https://asirikuy.com/newsite/asirikuyfiles/numpy.whl'
            r = requests.get(url, data=data, stream=True)
            check_output("pip install numpy.whl", shell=True)
        elif moduleName == 'fastcsv' and system == "Windows":
            check_output(r"copy fastcsv\*fast* C:\Python27\Lib\site-packages" + moduleName, shell=True)
        else:
            if system == "Windows":
                check_output("pip install " + moduleName, shell=True)
            elif system == "Linux":
                check_output("sudo pip install " + moduleName, shell=True)

