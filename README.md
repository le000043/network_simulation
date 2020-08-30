**NOTE:**Worked with yoonkwon-yi on this project
### Set up
**NOTE:**It is best to run on a Linux machine
1. Clone the repository to your local machine
2. Open a terminal 
3. To start on server, type 
```
make all
<Enter>
./bl_server server_name
```
### you can replace server_name by any names you choose to
**A snapshot of working server**
![Image of server_01](https://github.com/le000043/network_simulation/blob/master/images/server_01.png)
4. To start a client
5. Open a new terminal
6. type 
```
./bl_client server_name client_name
```
### using the same server_name you used at step 3. You can replace client_name with any names you choose to
Example of clients
![Image of client_01](https://github.com/le000043/network_simulation/blob/master/images/client_01.png)
7. Type anything on the client terminal and enter to finish
Schematics of clients sending normal messages
![Image of client_send_msg](https://github.com/le000043/network_simulation/blob/master/images/client_send_msg.png)
8. As client, to exit the current server, do the following:
```
Ctrl + D
```
Schematics of clients joining a server
![Image of schematics_2_client_join](https://github.com/le000043/network_simulation/blob/master/images/schematics_2_client_join.png)
Schematics of clients joining a server with 2 existing clients
![Image of clients_join_server_with_2_existing_clients](https://github.com/le000043/network_simulation/blob/master/images/clients_join_server_with_2_existing_clients.png)
Schematics of clients departing
![Image of client_departure](https://github.com/le000043/network_simulation/blob/master/images/client_departure.png)

