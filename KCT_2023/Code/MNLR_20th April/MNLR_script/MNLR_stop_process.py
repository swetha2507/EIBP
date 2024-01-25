import paramiko
import sys
import os
import time
import json
from xml.dom import minidom

def hostInfo(node):
    '''
    Extract the hostname and Port number under the node tag
    :return:
    '''
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
    print ("No of interface: {0}".format(len(int_lst)))
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
    #print "Number of Nodes: {0}".format(len(node_lst))
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

def stop(map, uname, password, keyLocation, remoteDst):
    
    server_address = []
    port_address = []
    
    for key,values in map.items():
        server_address.append(values[0])
        port_address.append(values[1])
    
    command1 = 'sudo pkill MNLR'
    command2= 'sudo rm -r ' + remoteDst + '/MNLR_Code/mnlrlogs'
    
    mykey = paramiko.RSAKey.from_private_key_file(keyLocation,password)
    # create an SSHClient object that we'll use to create an SSH session
    client = paramiko.SSHClient()
    # if local SSH keys do not exist, create a policy for logging a warning in Python
    client.set_missing_host_key_policy(paramiko.WarningPolicy())
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    # connect to the server using its address, port, username student, and privatekey
    for server,port in zip(server_address,port_address):
        client.connect(server, port=port, username=uname, pkey=mykey)
        # run the contents of command and save the output results of the command to variables
    # stdin, stdout, stderr = client.exec_command('rm -r /users/sv5269/*')
        stdin, stdout, stderr = client.exec_command(command1)
        stdin, stdout, stderr = client.exec_command(command2)
        time.sleep(1)

if __name__ == "__main__":
    
    file = "Credentials_Path.txt"
    fileop = open(file, 'r')
    file = json.loads(fileop.read())
    
    uname = file.get('uname')
    password = file.get('password')
    rspec_file = file.get('rspec_file')
    keyLocation = file.get('keyLocation')
    remoteDst = file.get('remoteDst')
    
    print(rspec_file)
    xmldoc = minidom.parse(rspec_file)
    keys = formKeys('node', 'client_id')
    vals = formValues('node', 'interface', 'address', 'mac_address')
    map = dict(zip( keys, vals))
    
    stop(map, uname, password, keyLocation, remoteDst)
    fileop.close()
    