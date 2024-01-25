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
from collections import defaultdict
import ipaddress

def buildDictonary(rspec):
    connectionInfo = {} # Holds connection and network information for nodes in RSPEC
    addressingInfo = defaultdict(list) # Temporary collection to hold network information

    # Parse RSPEC
    getConnectionInfo(rspec, connectionInfo, addressingInfo)
    getAddressingInfo(connectionInfo, addressingInfo) # Adds addressing info into connection dictonary

    return connectionInfo

def getConnectionInfo(rspec, connectionInfo, addressingInfo):
    xmlInfo = minidom.parse(rspec)

    # Get the top-level XML tag Node, which contains node information
    nodes = xmlInfo.getElementsByTagName('node')

    # Iterate through each node in the topology and grab its information
    for node in nodes:
        # Get name of node (ex: node-1)
        nodeName = node.attributes['client_id'].value

        # Get the login credentials for the node, which is the FQDN and port
        loginCreds = node.getElementsByTagName("login")
        hostname = (loginCreds[0].attributes["hostname"]).value
        port = (loginCreds[0].attributes["port"]).value

        # Each entry in the dictonary is a 3-tuple/triple containing the FQDN, port, and network information
        connectionInfo[nodeName] = (hostname, port, {})

        # Get the IP address and mask for each network interface attached to the node
        networkInterfaceInfo = node.getElementsByTagName("ip")
        for interface in networkInterfaceInfo:
            ipv4addr = (interface.attributes["address"]).value
            subnetMask = (interface.attributes["netmask"]).value

            # With the address and mask, determine the network the interface is attached to
            # example: 10.10.1.5/24 address --> 10.10.1.0/24 network
            fulladdr = "{}/{}".format(ipv4addr, subnetMask)
            networkAddress = ipaddress.ip_network(fulladdr, strict=False)

            # Store the networking info in the other dictonary for use later
            addressingInfo[networkAddress].append((nodeName, ipv4addr))

    return

def getAddressingInfo(connectionInfo, addressingInfo):
    # In the connection info, index 2 in the 3-tuple is the network information
    NETWORK_INFO_INDEX = 2

    # For each subnet found in the RSPEC
    for subnet in addressingInfo.items():
        subnetAddr = subnet[0]
        subnetInfo = subnet[1]

        # For each node attached to the subnet
        for node in subnetInfo:
            localNode = node[0]
            localAddr = node[1]

            # For each of the other nodes attached to the subnet
            for otherNode in subnetInfo:
                remoteNode = otherNode[0]
                remoteAddr = otherNode[1]
                if(remoteNode != localNode):
                    newInfo = {"remoteIPAddr": remoteAddr, "localIPAddr": localAddr, "subnet": subnetAddr}
                    connectionInfo[localNode][NETWORK_INFO_INDEX][remoteNode] = newInfo
   
    return
    
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
    
def orchestrateRemoteCommands(remoteGENINode, GENISliceDict, cmdList, getOutput=False, waitForResult=True):
    """
    Execution of a single or collection of commands on a GENI node.
    :param remoteGENINode: Name of the remote node
    :param GENISliceDict: Dictionary of GENI slice connection information
    :param cmdList: Either a string or string list of remote Bash commands
    :param getOutput: Return the output of the command
    :param waitForResult: Blocking call to wait for completion of command
    :return cmdOutput: Result of output
    :return: Return nothing
    """

    remoteShell = paramiko.SSHClient()
    remoteShell.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    mykey = paramiko.RSAKey.from_private_key_file(keyLocation, password = password)

    host, port = getGENIHostConnectionArgs(GENISliceDict, remoteGENINode)
    remoteShell.connect(host, username = uname, pkey = mykey, port = int(port))

    if(getOutput):
        if(type(cmdList) is str):
            stdin, stdout, stderr = remoteShell.exec_command(cmdList)
            cmdOutput = stdout.read()
            return cmdOutput
        else:
            sys.exit("not a valid command, please check what you are passing to the function")

    else:
        if(type(cmdList) is str):
            stdin, stdout, stderr = remoteShell.exec_command(cmdList)
            if(waitForResult):
                print("Result: {0}".format(getExitStatus(stdout.channel.recv_exit_status())))

        elif(type(cmdList) is list):
            print("#####SCRIPT COMMAND LIST#####")
            print(*cmdList, sep = "\n")
            print("#############################")

            for command in cmdList:
                print("sending command")
                stdin, stdout, stderr = remoteShell.exec_command(command)
                if(waitForResult):
                    print("Waiting for result")
                    print("Result: {0}".format(getExitStatus(stdout.channel.recv_exit_status())))

        else:
            sys.exit("not a valid command, please check what you are passing to the function")

    remoteShell.close()
    print("\n")

    if(getOutput):
        return cmdOutput

    else:
        return None

def searchPort(node,GENIDict):
    output = orchestrateRemoteCommands(node,GENIDict,"ifconfig",True).decode().strip().split("\n")
    output_dict = {}
    i = 0
    while i < len(output):
        if re.search(r'eth\d+:\sflag',output[i]):
            output_dict[output[i+1].strip().split(" ")[1].strip()] = output[i].split(":")[0].strip()
            i += 1
        i += 1

    return output_dict
    
def printDictonaryContent(GENISliceDict,searchPort=None):
    HOSTNAME = 0
    PORT = 1
    NET_INFO = 2

    for node in sorted(GENISliceDict):
        output_dict = ""
        if searchPort is not None:
            output_dict = searchPort(node,GENISliceDict)
        print("node: {}".format(node))
        print("\thostname: {}".format(GENISliceDict[node][HOSTNAME]))
        print("\tport: {}".format(GENISliceDict[node][PORT]))

        for neighbor in GENISliceDict[node][NET_INFO]:
            print("\tsubnet: {}".format(GENISliceDict[node][NET_INFO][neighbor]["subnet"]))
            print("\t\tlocal IP address: {}".format(GENISliceDict[node][NET_INFO][neighbor]["localIPAddr"]))
            print("\t\tneighbor: {}".format(neighbor))
            print("\t\tneighbor IP address: {}".format(GENISliceDict[node][NET_INFO][neighbor]["remoteIPAddr"]))
            if searchPort is not None:
                print("\t\tConnected port:",output_dict[GENISliceDict[node][NET_INFO][neighbor]["localIPAddr"]])


    return
    
if __name__ == "__main__":
    file = "Credentials_Path.txt"
    fileop = open(file, 'r')
    file = json.loads(fileop.read())
    
    uname = file.get('uname')
    password = file.get('password')
    keyLocation = file.get('keyLocation')
    remoteDst = file.get('remoteDst')
    localSrc = file.get('localSrc')
    cmd_file = file.get('cmd_file')
    rspec = file.get('rspec_file')

    GENIDict = buildDictonary(rspec)

    option = input("Include port? (yes/no): ")

    if len(option) == 0:
        printDictonaryContent(GENIDict)
    elif option[0] == 'n':
        printDictonaryContent(GENIDict)
    elif option[0] == 'y':
        printDictonaryContent(GENIDict,searchPort)
    else:
        print("Program ended")