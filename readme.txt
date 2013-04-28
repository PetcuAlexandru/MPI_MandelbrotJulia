PETCU ALEXANDRU-GABRIEL
333 CA

TEMA 3 APD

	Procesul Master:
		Mai intai citesc datele din fisierul de intrare(pe care le retin intr-o 
		structura de tip Data, in care mai retin si latimea si lungimea imaginii
		finale). Dupa citire determin care va fi lungimea si latimea matricii finale
		pentru care aloc memorie in pasul urmator. Trimit datele citite celorlalte
		procese si apoi calculez intervalul din matricea finala pe care va lucra 
		procesul Master.Am decis sa fac impartirea pe procese astfel: fiecare proces,
		inclusiv procesul Master, calculeaza un anumit numar(aprox egal) de linii din 
		matricea finala. Daca numarul de linii nu se imparte exact la numarul de procese
		atunci procesul master calculeaza cateva linii in plus fata de celelalte procese.
		Urmeaza generarea multimii Julia sau Mandelbrot pe intervalul determinat anterior,
		apoi creez o noua matrice in care primesc liniile calculate de celelalte procese.
		In functie de procesul de la care am primit matricea determin pozitia din care
		trebuie sa incep copierea liniilor in matricea finala.
		La final scriu ion fisierul de iesire matricea finala.
	Celelalte procese:
		Primesc de la master datele citite de acesta din fisier si lungimea si latimea
		imaginii finale, in functie de acestea determina intervalul pe care trebuie
		sa lucreze, salveaza rezultatul intr-o matrice pe care o trimite procesului
		master.

	Functii auxiliare pentru:
	-	suma a 2 numere complexe
	- produsul a 2 numere complexe
	- modulul unui numar complex
	- generarea multimii de tip Mandelbrot
	-	generarea multimii de tip Julia
	
	Am rulat tema pentru toate imaginile test variind numarul de procese si am
	obtinut rezultatele corecte(testate cu imgdiff32).
		
		
		
