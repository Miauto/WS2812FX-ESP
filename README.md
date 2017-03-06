# WS2812FX-ESP
Base on WS2812FX - Interface with Jeedom
[WS2812FX library](https://github.com/kitesurfer1404/WS2812FX)


les commandes pour piloté sont basé sur IP/Cmd (ex: 192.168.0.1/etat) :

- IP/set?c=FF0000
  - Couleur en HEX on peut mettre au choix
- IP/set?b=p
  - Augmente la luminosité
- IP/set?b=m
  - Diminue la luminosité
- IP/set?b=#slider#
  - On peut choisir la valeur entre 0-255
- IP/set?s=p
  - Augmente la vitesse
- IP/set?s=m
  - Diminue la vitesse
- IP/set?s=#slider#
  - On peut choisir la valeur entre 0-255
- IP/set?m=#slider#
  - nous permet de choisir un mode sur les 46 possible

 
Les informations :
- IP/etat
  - Donne une information 1/0 si la lampe est allumé
