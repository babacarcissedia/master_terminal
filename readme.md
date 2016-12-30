=== TERMINAL DISTRUBIÉ ===
execute command on every device (slaves)
display the result of the command on the master server

Two binary : 
- master
- slave



== Execution sample
* master :
	- bind, listen, accept for any incoming (just like reverse shell)
	- record connected client
	- send command
	- get result

* slave:
	- connect to master
	- receive command
	- send result

* slave and master have : 
	- thread for incoming message : 
	- thread for outgoing message



== Difficultés rencontrés

== Quelques remarques 
* l'appel de recv avec le drapeau MSG_WAITALL permet de recevoir l'intégralité des données
mais à l'inconvénient de bloquer lorsque la taille du message n'est pas encore atteinte
