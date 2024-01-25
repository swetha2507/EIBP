#!/usr/bin/env python
from GENIutils import *
import re

interfaceList = []
linkStatus = defaultdict(list)
addressing = []
outputAddressing = []

RSPEC = getConfigInfo("Local Utilities", "RSPEC")
GENIDict = buildDictonary(RSPEC)

getInterfaces = "ls /sys/class/net/"
addressingInfo = "ifconfig {} | grep -E 'inet|ether'"

f = open("topology.txt", "w+")

for currentRemoteNode in GENIDict:
    print(currentRemoteNode)
    interfaceList[:] = []

    f.write("---------------------{}---------------------\n".format(currentRemoteNode))

    interfaceList = orchestrateRemoteCommands(currentRemoteNode, GENIDict, getInterfaces, getOutput=True)
    interfaceList = str(interfaceList, 'utf-8').split("\n")
    interfaceList[:] = [interface for interface in interfaceList if "eth" in interface and "eth0" not in interface]

    for interface in interfaceList:
        f.write("{}:\n".format(interface))

        # ADDRESSING INFO
        updated_addressingInfo = addressingInfo.format(interface)
        RSTPOutput = orchestrateRemoteCommands(currentRemoteNode, GENIDict, updated_addressingInfo, getOutput=True)
        RSTPOutput = str(RSTPOutput, 'utf-8')
        IPAddr = re.search(r'inet(.*?)netmask', RSTPOutput).group(1).strip()
        f.write("{}\n".format(RSTPOutput))

        linkStatus[IPAddr].append(currentRemoteNode)
        linkStatus[IPAddr].append(interface)
        addressing.append(IPAddr)

for address in linkStatus:
    localValue = ""
    remoteValue = ""

    for addr in addressing:
        if(address.split(".")[2] == addr.split(".")[2]):
            if(address == addr):
                localValue = address
            else:
                remoteValue = addr

    if(localValue and remoteValue):
        outputAddressing.append("{0}_{1}--------{2}_{3}\n".format(linkStatus[localValue][0],linkStatus[localValue][1],linkStatus[remoteValue][1],linkStatus[remoteValue][0]))

outputAddressing.sort()
nodeHeader = outputAddressing[0].split("_")[0]

f.write("--------{}--------\n".format(nodeHeader))

for entry in outputAddressing:
    currentNode = entry.split("_")[0]
    if(currentNode != nodeHeader):
        nodeHeader = currentNode
        f.write("\n--------{}--------\n".format(nodeHeader))
    f.write(entry)
f.close()
