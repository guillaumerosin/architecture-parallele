//Analogie simple pour comprendre
// Imagine une boite aux lettres
int x = 42;      //la boite elle-meme (contient une valeur)
int *A = &x;     // l'adresse de la boite au lettre
int **B = &A;    // un bout de papier qui indique où trouver l'adresse de la boite

printf("%d\n", *A);   // → 42  (on suit A → x)
printf("%d\n", **B);  // → 42  (on suit B → A → x)
printf("%d\n", *B == A); 



// Quand utilise - t - on int**
// Le double pointeur est utile dans deux cas typiques 
// Modifier un pointeur depuis une fonction --> si tu passes int *A à une fonction,  la fonction peut modifier la valeur
// pointée, mais pas le pointeur lui même. Avec int **B, elle peut changer où A pointe
// Les tableaux 2D dynamiques : un tableau de tableaux s'implémente ouvent comme int**, chaque * correspondant à un niveau d'indexation (matrice[i][j])