#This program parses the xml topo file and creates a readable Topo file

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

def collect_info(info_file, user, psswd, keyLocation, localSrc, remoteDst ):
    my_dict = eval(open(info_file).read())
	
    print ("Number of Nodes: {0}".format(len(my_dict)))
    print ("RIT username : {0}".format(user))

    #try to copy the latest MNLR code from the execution server to each of the
    #nodes in the GENI

    for node in my_dict:
        if re.match('^node',node): #Filtering out to just nodes, thereby eliminating ipnodes configuration
            conn_info = my_dict[node]
          
            host, port = getGENIHostConnectionArgs(my_dict, node)
            transport = paramiko.Transport((host, int(port)))
            mykey = paramiko.RSAKey.from_private_key_file(keyLocation, password = psswd)
            
            transport.connect(username=user, pkey=mykey)
            SFTP = paramiko.SFTPClient.from_transport(transport)
            
            if(os.path.isdir(localSrc)):
                os.chdir(os.path.split(localSrc)[0])
                parent = os.path.split(localSrc)[1]
    
                for walker in os.walk(parent):
                    try:
                        print(SFTP.mkdir(os.path.join(remoteDst, walker[0]).replace("\\", "/"))) # Windows POSIX change
                    except:
                        pass
    
                    for file in walker[2]:
                        SFTP.put(os.path.join(walker[0], file).replace("\\", "/"), os.path.join(remoteDst, walker[0], file).replace("\\", "/")) # Windows POSIX change (src, dst)
    
            else:
                fullDest = os.path.join(remoteDst, os.path.basename(localSrc)).replace("\\", "/") # Windows POSIX change
                SFTP.put(localSrc, fullDest)
            
            SFTP.close()
            transport.close()

    return my_dict

def connect_args(full_dict, node):
    '''
    It Returns three parameters
    :param full_dict:
    :return: hostname, port
    '''
    tmp_host = full_dict[node]
    host,port = tmp_host[0],tmp_host[1]
    
    return host,port

def trigger_MNLR(info_dict, user, pswd, keyLocation, cmd_file, remoteDst):

    '''
    Execute appropriate MNLR commands on each nodes
    :return:
    '''
    #read the command file
    nodes = []
    for node in info_dict:
        if re.match('^node',node): nodes.append(node)  #Filtering out to just nodes, thereby eliminating ipnodes configuration 
        

    node_nam = []
    node_cmd = []
    fileop = open(cmd_file, 'r')
    cmd_lst = fileop.read().splitlines()

    print (cmd_lst)

    for i in cmd_lst:
        tmp_i = i.strip()
        cmd_split = tmp_i.split(',')
        node_nam.append(cmd_split[0])
        node_cmd.append(cmd_split[1])

    #Forming Cmd Dictionary
    Mcmd_dict = dict(zip(node_nam, node_cmd))

    #Maintain a list for the hostnames
    ssh = []

    for i in range(0, len(nodes)):
        ssh.append(paramiko.SSHClient())
        tmp_info = info_dict[node]
        ssh[i].set_missing_host_key_policy(paramiko.AutoAddPolicy())
        mykey = paramiko.RSAKey.from_private_key_file(keyLocation,password=password)
        sys.stdout.write("\rConnecting: %s" % (tmp_info[0]))
        print(tmp_info[0][1])
        sys.stdout.flush()

        #call the command args function
        host, port = connect_args(info_dict, nodes[i])
        ssh[i].connect(host, username= user, pkey=mykey, port=int(port))

        if  not  nodes[i][:-1] == 'ipnode': #ipnode or host or IPN

            stdin, stdout, stderr = ssh[i].exec_command('rm -rf mnlrlogs', timeout=30)
            showtime = strftime("%m%d%H%M%Y.%S", localtime()) #we get the current time in format MMDDhhmmYYYY.SS
      
            #check tail -f logs in the folder where you trigger MNLR
            stdin, stdout, stderr = ssh[i].exec_command('sudo pkill MNLR', timeout=30)
            stdin, stdout, stderr = ssh[i].exec_command('uptime')
            stdin, stdout, stderr = ssh[i].exec_command('cd '+ remoteDst +'/MNLR_Code/;gcc -g -o MNLR *.c -lm', timeout=30)
            time.sleep(2)
            stdin, stdout, stderr = ssh[i].exec_command('sudo sysctl -w net.ipv4.ip_forward=0', timeout=30) #cat /proc/sys/net/ipv4/ip_forward            
            time.sleep(2)
            
            stdin, stdout, stderr = ssh[i].exec_command('pwd', timeout=30)           
            x ='cd ' + remoteDst + '/MNLR_Code/; sudo screen -dmS ' + str(nodes[i]) + ' -L -Logfile mnlrlogs-'+str(nodes[i])+' '+ str(Mcmd_dict[nodes[i]])
            print(x)
            stdin, stdout, stderr = ssh[i].exec_command(x)

            sys.stdout.write("\r MNLR is Running on :%s\n" % (nodes[i]))
            ssh[i].close()

            print ("cmd : {}".format(Mcmd_dict[nodes[i]]))

        else:
            print ("{0} is a IP node".format(nodes[i]))
            stdin, stdout, stderr = ssh[i].exec_command('sudo sysctl -w net.ipv4.ip_forward=0', timeout=30)
            print ("IP forwarding disabled on", format(nodes[i]))
            ssh[i].close()


        print ('Cleared the Existing Stale configurations...')

def getGENIHostConnectionArgs(GENISliceDict, node):
    """
    Gets the FQDN and port used by a GENI node in a given GENI slice via a custom dictonary.

    Args:
        GENI_dict (dict): A dictonary comprised of information about GENI nodes in a slice. The dictonary must be created with buildDictonary() to be valid.
        node (str): The hostname of a given GENI node (example: node-1)

    Returns:
        string (host): The FQDN of the requested GENI node.
        integer (port): The port number of the requested GENI node.
    """

    connectionInfo = GENISliceDict[node]
    host, port = connectionInfo[0], connectionInfo[1]

    return host, port

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
    
    print(rspec_file)
    xmldoc = minidom.parse(rspec_file)
    keys = formKeys('node', 'client_id')
    vals = formValues('node', 'interface', 'address', 'mac_address')
    map = dict(zip( keys, vals))
    time_now = strftime("%Y-%m-%d-%H:%M:%S", gmtime())
    info_file = dumpInFile(map, time_now)

    info_dict = collect_info(info_file, uname, password, keyLocation, localSrc, remoteDst)

    trigger_MNLR(info_dict, uname, password, keyLocation, cmd_file, remoteDst)
    fileop.close()
    