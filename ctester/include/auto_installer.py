
from subprocess import check_output
import platform

def installIfNeeded(moduleName, nameOnPip=None, notes="", log=print):
    """ Installs a Python library using pip, if it isn't already installed. """
    import sys
    system = platform.system()

    # Check if the module is installed by trying to import it
    try:
        __import__(moduleName)
        # Module is already installed
        return
    except ImportError:
        pass  # Module not installed, continue with installation
    
    log("Installing " + moduleName + notes + " Library for Python")
    if moduleName == 'numpy' and system == "Windows":
        try:
            import requests
            user = input("Please enter your asirikuy username")
            password = input("Please enter your asirikuy password")
            data = {'username': user, 'password': password}
            url = 'https://asirikuy.com/newsite/asirikuyfiles/numpy.whl'
            r = requests.get(url, data=data, stream=True)
            check_output("python3 -m pip install numpy.whl", shell=True)
        except ImportError:
            # If requests is not available, fall back to standard pip install
            check_output("python3 -m pip install " + moduleName, shell=True)
    elif moduleName == 'fastcsv' and system == "Windows":
        # fastcsv is a local module in include/fastcsv, no installation needed
        # Just skip it - the module is already available locally
        pass
    else:
        if system == "Windows":
            # Use python3 -m pip to ensure we install to the correct Python 3 environment
            check_output("python3 -m pip install " + moduleName, shell=True)
        elif system == "Linux":
            check_output("sudo python3 -m pip install " + moduleName, shell=True)

