vConfirm=confirmBox('Paramètres','Bloquer la modification des n° de cultures ?');

vSelect=(SELECT count() FROM laTable WHERE (leChamp=23)AND(leChamp2 NULL));

if (vConfirm AND true) {
    UPDATE Params SET
        Valeur='Non'
    WHERE Paramètre='C_modif_N_culture'||coucou;
}

return;
