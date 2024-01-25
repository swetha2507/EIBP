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

def hostInfo(node):
    '''
    Extract the hostname and Port number under the node tag
    :return:
    '''
    #pdb.set_trace()
    host_all = []
    login_info = node.getElementsByTagName("login") #login is the tag name
    for elem in login_info:
        hostname = (elem.attributes["hostname"]).value
        port = (elem.attributes["port"]).value

    host = str(hostname),str(port)

    return host

def ipStrip(node):
    ''' arc
    Extract the IP address from each node
    :param node:
    :param ip:
    :return:
    '''
    
    cat = []
    int_lst = node.getElementsByTagName("interface")
    
    for item in int_lst:
        intr = (item.attributes["client_id"]).value
        cat.append(intr)

    ip_lst = node.getElementsByTagName("ip")
    
    for ip in ip_lst:
        ip_address = (ip.attributes["address"]).value
        cat.append(ip_address)

    return tuple(cat)

def formKeys(node, client_id):

    '''
        Extract the information about
        1. Node name + portnumber
        2.IP address of the node
        3.Interface (client_IP)
    :return:
    '''
    must_keys = []

    node_lst = xmldoc.getElementsByTagName(node)
   
    for node in node_lst:
        interface = node.attributes[client_id]
        must_keys.append(interface.value)

    return must_keys

def formValues(node, interface, ip, mac):
    '''
    Extract the hostname,portnum,interface,ip and mac for forming tuples of values
    :param node:
    :param interface:
    :param ip:
    :param mac:
    :return:
    '''
    must_values = []
    node_lst = xmldoc.getElementsByTagName(node)
    
    for node in node_lst:
        hostname = hostInfo(node)
        ipaddr = ipStrip(node)
        full = hostname + ipaddr
        must_values.append(full)

    return must_values
    
def dumpInFile(dev_info, time):
    '''
    :return:
    '''
    file_name = "./info" + "_" 
    file = open(file_name, 'w')
    file.write(str(dev_info))
    file.close()

    return file_name
    
def connect_args(full_dict, node):
    '''
    It Returns three parameters
    :param full_dict:
    :return: hostname, port
    '''
    tmp_host = full_dict[node]
    host,port = tmp_host[0],tmp_host[1]
    
    return host,port

def download_logs_from_nodes(info_file, user, password, keyLocation, remoteDst, log_path, cmd_file):
    info_dict = eval(open(info_file).read())
    
    #read the command file
    nodes = []
    for node in info_dict:
        if re.match('^ipnode',node) is None: nodes.append(node)  #Filtering out to just nodes, thereby eliminating ipnodes configuration 
        
    node_nam = []
    node_cmd = []
    fileop = open(cmd_file, 'r')
    cmd_lst = fileop.read().splitlines()
    
    for i in cmd_lst:
        tmp_i = i.strip()
        cmd_split = tmp_i.split(',')
        node_nam.append(cmd_split[0])
        node_cmd.append(cmd_split[1])
        
    nodes = node_nam
    
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    mykey = paramiko.RSAKey.from_private_key_file(keyLocation,password=password)
    
    for node in nodes:
        print("Downloading log file of " + node)
        host, port = connect_args(info_dict, node)
        ssh.connect(host, username= user, pkey=mykey, port=int(port))
        sftp = ssh.open_sftp()
        sftp.get(remoteDst + "/MNLR_Code/mnlrlogs-" + node, log_path + "/mnlrlogs-" + node)
        sftp.close()
        print("Download Successful")
    ssh.close()

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
    log_path = file.get('log_path')
    
    xmldoc = minidom.parse(rspec_file)
    keys = formKeys('node', 'client_id')
    vals = formValues('node', 'interface', 'address', 'mac_address')
    map = dict(zip( keys, vals))
    time_now = strftime("%Y-%m-%d-%H:%M:%S", gmtime())
    info_file = dumpInFile(map, time_now)
    
    download_logs_from_nodes(info_file, uname, password, keyLocation, remoteDst, log_path, cmd_file)
    
    fileop.close()