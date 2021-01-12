// -------- Prérequis -------- //

Notre programme est utilisable uniquement sous Linux, personnellement nous utilisons le wsl Kali Linux.


// -------- Lancer le Serveur -------- //

Il faut se placer au bon endroit avec la commande : `cd /mnt/c/Desktop/ProjetSysteme` (cela depend du chemin d'accès)

Puis creer le fichier .exe server avec la commande : `make`

Ensuite se placer au bon endroit pour pouvoir compiler les fichiers .exe : `cd bin`

Il ne reste plus qu' à lancer le serveur avec la commande : `./server 3600 sauvegarde.sdat` où 3600 est un exemple de port utilisable 

et sauvegarde.sdat est un exemple de fichier de sauvegarde (non obligatoire)


// -------- Lancer un Client -------- //

Il faut se placer au bon endroit avec la commande : `cd /mnt/c/Desktop/ProjetSysteme` (cela depend du chemin d'accès)

Puis creer le fichier .exe client avec la commande : `make`

Ensuite se placer au bon endroit pour pouvoir compiler les fichiers .exe : `cd bin`

Il ne reste plus qu' à lancer le client avec la commande : `./client 127.0.0.1 3600` où 3600 est le port renseigné dans le serveur
