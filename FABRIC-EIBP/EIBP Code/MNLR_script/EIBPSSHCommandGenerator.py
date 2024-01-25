import pdb
import paramiko
import sys
import os
import re
import time
import subprocess
import json
from time import localtime, strftime, gmtime
from xml.dom import minidom

def create(rspec):
    xmlInfo = minidom.parse(rspec)
    # Get the top-level XML tag Node, which contains node information
    nodes = xmlInfo.getElementsByTagName('node')

    nodes_cmd_collection = []
    for node in nodes:
        cmd_component_list = []
        nodeName = node.attributes['client_id'].value.upper()
        cmd_component_list.append(nodeName) # node name
        loginCreds = node.getElementsByTagName("login")
        hostname = (loginCreds[0].attributes["hostname"]).value
        port = (loginCreds[0].attributes["port"]).value
        cmd_component_list.append(hostname) # hostname
        cmd_component_list.append(port) # port number

        nodes_cmd_collection.append(cmd_component_list)
    
    nodes_cmd_collection.sort(key=lambda x:x[0])
    return nodes_cmd_collection
    
if __name__ == "__main__":
    file = "Credentials_Path.txt"
    fileop = open(file, 'r')
    file = json.loads(fileop.read())
    
    uname = file.get('uname')
    password = file.get('password')
    rspec_file = file.get('rspec_file')
    keyLocation = file.get('keyLocation')
    remoteDst = file.get('remoteDst')
    localSrc = file.get('localSrc')
    cmd_file = file.get('cmd_file')

    xmldoc = minidom.parse(rspec_file)

    collection = create(rspec_file)
    
    print()
    
    for item in collection:
        print(item[0]+":")
        print("ssh {0}@{1} -p {2} -i {3}\n".format(uname,item[1],item[2],keyLocation))
    
    