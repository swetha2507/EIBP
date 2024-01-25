uthor__ = 'Pranav'
__author__ = 'sai'

##########

import paramiko
import time
import sys


#Inputs to the script
# S`ecify Code Path in the Linux machine
codePath = '/users/saiprash/test_build/';
dataPath = '/users/saiprash/test_build/data/';
localPath = '/Users/saiprash/Desktop/mnlr_data/'

# Specify Number of MNLR nodes to SSH
numOfMNLRNodes = 15

# Specify Number of IP Domain nodes to SSH
numOfIPNodes = 2

# Topology Name
topologyName = '3tiertop'

# Assign Node Tier Configurations
mnlrTierAddress = []
mnlrTierAddress.append('1.1')               #node1
mnlrTierAddress.append('1.2')               #node2
mnlrTierAddress.append('1.3')               #node3
mnlrTierAddress.append('2.1.1')             #node4
mnlrTierAddress.append('2.1.2')             #node5
mnlrTierAddress.append('2.2.1')             #node6
mnlrTierAddress.append('2.2.2,2.3.1')       #node7
mnlrTierAddress.append('2.3.2')             #node8
mnlrTierAddress.append('2.3.3')             #node9
mnlrTierAddress.append('3.1.1.1')           #node10
mnlrTierAddress.append('3.1.1.2,3.1.2.1')   #node11
mnlrTierAddress.append('3.2.1.1')           #node12
mnlrTierAddress.append('3.2.2.1,3.3.1.1')   #node13
mnlrTierAddress.append('3.3.2.1,3.3.3.1')   #node14
mnlrTierAddress.append('3.3.3.2')           #node15


# Assign End Network Node IDS
endNWNodeIDS =[11, 13];
endNWIPS = ['10.15.21.2', '10.12.22.2'];
endNWCIDR = ['24','24','24']
endInterfaces =[];
endNodeSSH = [];

# Create ssh sessions for numOfMNLRNodes.
ssh = []
for i in range (0, numOfMNLRNodes):
    ssh.append(paramiko.SSHClient())

# Connect to all MNLR Nodes and perform a cleanup
hostNames=[]
track = 0
for i in range(0, numOfMNLRNodes):
    ssh[i].set_missing_host_key_policy(paramiko.AutoAddPolicy())
    hostNames.append('node'+ str(i+1) + '.' + topologyName + '.fct.emulab.net');
    sys.stdout.write("\rConnecting: %s" % hostNames[i])
    sys.stdout.flush();
    #Enter the emulab linux instance credentials to loginto the machine
    ssh[i].connect(hostNames[i], port=22, username='saiprash', password='Amigo!');

    sys.stdout.write("\rConnected: %s" % hostNames[i]);
    sys.stdout.flush();

    # Cleanup the existing previous configurations
    stdin, stdout, stderr = ssh[i].exec_command('sudo pkill tshark');
    stdin, stdout, stderr = ssh[i].exec_command('sudo pkill hello');
    stdin, stdout, stderr = ssh[i].exec_command('ps -ef | grep hello');
    stdin, stdout, stderr = ssh[i].exec_command('ps -ef | grep tshark');
    stdin, stdout, stderr = ssh[i].exec_command('rm -f' + dataPath + '*');

    print 'Cleared the Existing Stale configurations...'

# Connect each MNLR Node, and run configurations
#Modified pranav's function to make it work for the multi tier topologies
for i in range(0, numOfMNLRNodes):

    # Check interfaces where tshark has to be run
    stdin, stdout, stderr = ssh[i].exec_command('awk \'{print $1,$2}\' /var/emulab/boot/ifmap')
    output = stdout.readlines();

    

    #consctruct a tshark command 
    tcmd = 'sudo tshark'
    for j in range(0, len(output)):
        temp = output[j].split(' ');
        if temp[1].rstrip() not in endNWIPS:
            tcmd += ' -i ' + temp[0];
        else:
            # Store end n/w interfaces.
            endInterfaces.append(temp[0]);

            # Store end n/w SSH details, have to change this if you have multiple end networks for each node.
            endNodeSSH.append(ssh[i]);

            #print "Ignoring end network" + temp[0] + " as it is a control IP"
            sys.stdout.flush();
    #Adding the pcap capture command for clear analysis
    tcmd += ' -w '+ dataPath + 'node' + str(i+1) + '.pcap';
    print 'exec# ' + tcmd;

    # start tshark
    stdin, stdout, stderr = ssh[i].exec_command(tcmd);

    # build MNLR command
    mcmd ='sudo '+ codePath+ 'hello';

    # append tierAddrs
    tiers = mnlrTierAddress[i].split(',');

    temp1 = len(tiers)
    if (temp1 > 0):
        mcmd += ' -T';
        for j in range (0, len(tiers)):
            mcmd += ' ' + tiers[j];

    # check if node is an end node
    if (i + 1) in endNWNodeIDS:
        mcmd += ' -N 0';
        print track
        print endNWIPS[track]
        print endNWCIDR[track]
        print endInterfaces[track]
        mcmd += ' ' + endNWIPS[track] + ' ' + endNWCIDR[track] + ' ' + endInterfaces[track];
        track +=1;
    else:
        mcmd += ' -N 1';

    mcmd += ' > /dev/null';
    print 'exec# '+ mcmd;

    # start mnlr_start
    stdin, stdout, stderr = ssh[i].exec_command(mcmd);

# Wait few seconds to stabilize
time.sleep(10)
sys.stdout.flush();
sys.stdout.write("\rWaiting for all nodes to stabilize...")
time.sleep(10);
sys.stdout.flush();
sys.stdout.write("\rDone.\n")

# Bring down one of the end network.
print 'Bringing down interface now ' + endInterfaces[0];
stdin, stdout, stderr = endNodeSSH[0].exec_command('sudo ip link set dev ' + endInterfaces[0] + ' down')
time.sleep(5);

# Bring up
print 'Bringing up interface now ' + endInterfaces[0];
stdin, stdout, stderr = endNodeSSH[0].exec_command('sudo ip link set dev ' + endInterfaces[0] + ' up')
time.sleep(5);

# Bring down one of the end network.

print 'Bringing down interface second time' + endInterfaces[0];
stdin, stdout, stderr = endNodeSSH[0].exec_command('sudo ip link set dev ' + endInterfaces[0] + ' down')
time.sleep(5);

# Bring up
print 'Bringing up interface second time' + endInterfaces[0];
stdin, stdout, stderr = endNodeSSH[0].exec_command('sudo ip link set dev ' + endInterfaces[0] + ' up')
time.sleep(5);

# status
sys.stdout.flush();
sys.stdout.write("Converting captured pcaps to .csv");
print ''
for i in range(0, numOfMNLRNodes):
    pcap2csvcmd = 'sudo tshark -r ' + dataPath + 'node' +  str(i+1) +'.pcap -T fields -E separator=, -e frame.time_epoch -e frame.interface_id -e eth.type -e frame.number -e data.data > ' + dataPath + 'node' + str(i+1) +'.csv';
    print pcap2csvcmd
    stdin, stdout, stderr = ssh[i].exec_command(pcap2csvcmd)

sys.stdout.flush();
sys.stdout.write("\rConverted captured pcaps to .csv");

print('70 percent done...');

# copy all remote .csv files to local folder.
transport = paramiko.Transport((hostNames[0], 22));
password = "Amigo!"
username = 'saiprash'
transport.connect(username=username, password=password)
sftp = paramiko.SFTPClient.from_transport(transport);
stdin, stdout, stderr = ssh[0].exec_command('sudo chmod 777' + dataPath + "*" )
for i in range(0 , numOfMNLRNodes):

    sftp.get(dataPath+'node'+str(i+1)+'.csv', localPath +'node'+str(i+1)+'.csv')

sftp.close()

# Merge all .csv files to one file.
sys.stdout.flush();
#final path file
filePath = localPath+'final.csv'
sys.stdout.write('\rMerging... .csv files to single ' + filePath);

data =[]
#create a csv file and open 
with open(filePath, 'w') as outfile:
    for i in range(0, numOfMNLRNodes):
        with open(localPath+'node' + str(i+1) + '.csv') as infile:
            for line in infile:
                line = str(i+1) + ',' + line
                lineArray = line.split(',');

                dataArray = lineArray[5].split(':')

                # is MNLR type 5
                if dataArray[0] == '05':
                    if dataArray[1] == '01' or dataArray[1] == '02':
                        data.append(line)
                        outfile.write(line)




sys.stdout.flush();
sys.stdout.write('\rMerged .csv files to single ' + filePath);

print ''

print 'Job Done! Congrats!'

