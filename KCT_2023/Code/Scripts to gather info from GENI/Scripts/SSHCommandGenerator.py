from xml.dom import minidom
from GENIutils import getConfigInfo

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


def main():
    username = getConfigInfo("GENI Credentials", "username")
    keyLocation = getConfigInfo("Local Utilities", "privateKey")
    rspec = getConfigInfo("Local Utilities", "RSPEC")

    collection = create(rspec)

    for item in collection:
        print(item[0]+":")
        print("ssh {0}@{1} -p {2} -i {3}\n\n".format(username,item[1],item[2],keyLocation))    



if __name__ == "__main__":
    main()
    
    