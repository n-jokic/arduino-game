#Praktikum iz merno-akvizicionih sistema - Drugi domaci

from threading import Thread
from PyQt5.QtWidgets import QApplication, QPushButton, QLineEdit, QLabel, QWidget
import numpy as np
import serial
import sys
import time

arduino = serial.Serial("COM14", 9600)
time.sleep(1)
reset = 0
arduinoData = 3
fileName = ""
name1 = ""
name2 = ""
board = np.full((3,3), 10)
gameStatus = 2


#GUI
class App(QWidget):
    
    #Interferjs se sastoji od 3 button-a i 3 textbox-a za unos podataka
    #Prvi button je Start button koji se koristi za pokretanje svega
    #Drugi button je Next button koji se koristi za prelazenje u sledecu partiju kad se prethodna zavrsi
    #Treci button je Exit button koji sluzi za gasenje aplikacije
    
    def __init__(self):
        super().__init__()
        self.title = "Iks Oks"
        self.left = 200
        self.top = 200
        self.width = 320
        self.height = 170
        self.initUI()
        
    def initUI(self):
        self.setWindowTitle(self.title)
        self.setGeometry(self.left, self.top, self.width, self.height)
        
        self.labelName1 = QLabel('Ime prvog igraca:', self)
        self.labelName1.resize(200, 30)
        self.labelName1.move(10, 10)
        
        self.txtInputName1 = QLineEdit("", self)
        self.txtInputName1.resize(200, 30)
        self.txtInputName1.move(110, 10)
        
        self.labelName2 = QLabel('Ime drugog igraca:', self)
        self.labelName2.resize(200, 30)
        self.labelName2.move(10, 50)
        
        self.txtInputName2 = QLineEdit("", self)
        self.txtInputName2.resize(200, 30)
        self.txtInputName2.move(110, 50)
        
        self.labelFileName = QLabel('Ime fajla:', self)
        self.labelFileName.resize(200, 30)
        self.labelFileName.move(10, 90)
        
        self.txtInputFileName = QLineEdit("", self)
        self.txtInputFileName.resize(200, 30)
        self.txtInputFileName.move(110, 90)
        
        self.buttonStart = QPushButton("Zapocni Igru", self)
        self.buttonStart.resize(100, 30)
        self.buttonStart.move(10, 130)
        self.buttonStart.clicked.connect(self.buttonStart_fcn)
        
        self.buttonNext = QPushButton("Sledeca Igra", self)
        self.buttonNext.resize(100, 30)
        self.buttonNext.move(110, 130)
        self.buttonNext.clicked.connect(self.buttonNext_fcn)
        
        self.buttonStop = QPushButton("Izlaz", self)
        self.buttonStop.resize(100, 30)
        self.buttonStop.move(210, 130)
        self.buttonStop.clicked.connect(self.buttonStop_fcn)
        
        self.show()
    
    #Kad se pritisne Start button citamo informacije iz textbox-ova, cuvamo ih, i 
    #disablujemo sve button-e i textbox-ove dok se ne zavrsi trenutna partija
    def buttonStart_fcn(self):
        global fileName, name1, name2
        fileName = self.txtInputFileName.text()
        name1 = self.txtInputName1.text()
        name2 = self.txtInputName2.text()
        self.buttonStart.setEnabled(False)
        self.buttonNext.setEnabled(False)
        self.buttonStop.setEnabled(False)
        self.txtInputFileName.setEnabled(False)
        self.txtInputName1.setEnabled(False)
        self.txtInputName2.setEnabled(False)
        print("Arduinu poslat Start")
        t2.start()
        t3.start()
        arduino.write('0'.encode())
    
    #Kad se Stop button pritisne, izlazimo iz aplikacije
    def buttonStop_fcn(self):
        arduino.close()
        sys.exit()
     
    #Kad se Next button pritisne, pokrecemo opet igru
    def buttonNext_fcn(self):
        global gameStatus
        self.buttonNext.setEnabled(False)
        self.buttonStop.setEnabled(False)
        gameStatus = 2
    

        
#Ova funkcija se koristi za proveru toga da li imamo pobednika 
#Na pocetku imamo matricu 3x3 sa svim poljima 10, proveru da li je neko pobedio
#Mozemo da radimo na nekoliko razlicitih nacina, jedan nacin je da gledamo srednju vrednost
#Kolone/reda sto je zgodno zbog ugradjene funkcije, a drugi nacin je gledati ukupan zbir, sto moramo
#Da radimo za dijagonale
def checkForWinner(matrix):
    global win1, win2
    for i in range(3):
        if np.mean(matrix, 0)[i] == 0:
            return 0
        if np.mean(matrix, 1)[i] == 0:
            return 0
    for i in range(3):
        if np.mean(matrix, 0)[i] == 1:
            return 1
        if np.mean(matrix, 1)[i] == 1:
            return 1
    if board[0, 0] + board[1, 1] + board [2, 2] == 3:
        return 1
    if board[0, 0] + board[1, 1] + board [2, 2] == 0:
        return 0
    if board[0, 2] + board[1, 1] + board [2, 0] == 3:
        return 1
    if board[0, 2] + board[1, 1] + board [2, 0] == 0:
        return 0
    return 2
    
#Ova funkcija proverava da li je izabrano polje vec zauzeto, vraca 1 ako jeste, 0 ako nije
def checkIfClaimed(row, column, matrix):
    if matrix[row, column] != 10:
        return 1
    return 0

#Ova funkcija proverava da li je cela tabla puna, to najlakse radimo tako sto trazimo maksimum matrice
#A ako je on manji od 10, znaci da je puna
def allClaimed(matrix):
    if np.max(matrix) != 10:
        return 1
    return 0


#Ovo je thread za GUI
def fun1():
    global ex
    app = QApplication(sys.argv)
    ex = App()
    app.exec_()
    
t1 = Thread(target = fun1)


#Thread za komunikaciju sa Arduinom
def fun2():
    global arduinoData
    while True:
        val = arduino.readline()
        if int(val) == 0:
            arduinoData = 0
        if int(val) == 1:
            arduinoData = 1
        if int(val) == 2:
            arduinoData = 2

t2 = Thread(target = fun2)

#Ovom funkcijom Arduinu saljemo informaciju o trenutnom igracu            
def sendCurrentPlayerToArduino(currentPlayer):
    poruka = "Player" + str(currentPlayer)
    print(poruka)
    if currentPlayer == 1:
        arduino.write('3'.encode())
    if currentPlayer == 2:
        arduino.write('4'.encode())
   
#Ova funkcija upisuje rezultate u zeljeni file
def writeResultsToFile():
    global win1, win2, fileName, name1, name2
    f = open(fileName, 'w')
    f.write('Rezultat je:\n')
    f.write(name1)
    f.write(':\t')
    f.write(str(win1))
    f.write('\n')
    f.write(name2)
    f.write(':\t')
    f.write(str(win2))
    f.close()

#Thread za igru
def igra():
    global play
    global gameStatus
    global ex
    global arduinoData
    global board
    player = 1
    gameStatus = 2
    #Celu funkciju igra drzimo u zasebnom threadu
    while True:
        
        #gameStatus nam je indikator trenutnog stanja igre, gameStatus = 2 je indikator da se igra igra
        #0, 1 i 3 da je igra zavrsena i nova nece zapoceti dok se ne pritisne Next button
        while gameStatus == 2:
            
             #Ako je ceo board full a nismo imali pobednika, to znaci da je igra neresena
             if allClaimed(board) == 1:
                 print("Igra neresena")
                 board = np.full((3,3),10)
                 player = 1
                 gameStatus = 3
                 ex.buttonNext.setEnabled(True)
                 ex.buttonStop.setEnabled(True)        
             #Ako board nije pun, nastavljamo sa trenutnom igrom
             else:
                 #Proveravamo koji je igrac na redu, i u zavisnosti od indikatora obradjujemo njegov potez
                 if player == 1:
                    #Saljemo Arduinu da upali LED za prvog igraca
                    sendCurrentPlayerToArduino(player) 
                    #Ovde imamo praznu while petlju koja ceka informaciju od Arduina koju dobijamo iz druge niti
                    while arduinoData == 3:
                        pass
                    #Kada nam stigne informacija, cuvamo je u odgovarajucoj promenljivoj i opet radimo isto za sledecu informaciju
                    x1 = arduinoData
                    arduinoData = 3
                    while arduinoData == 3:
                        pass
                    y1 = arduinoData
                    arduinoData = 3
                    
                    #Kad su uneti i red i kolona proveravamo da li je izabrano polje zauzeto
                    #Ako jeste zauzeto ispisujemo poruku da je zauzeto i vracamo se na pocetak unosa polja
                    #Za tog igraca
                    if checkIfClaimed(y1, x1, board) == 1:
                        print("Izabrano polje je vec zauzeto, unesite ponovo")
                    #Ako izabrano polje nije zauzeto, upisujemo u tablu znak igraca i proveravamo da li imamo pobednika
                    else:
                        board[y1, x1] = 1
                        print(board)
                        player = 2
                        gameStatus = checkForWinner(board)
                    #Ako iz provere dobijemo da je igrac pobedio, saljemo podatak o tome Arduinu i zavrsavamo trenutnu partiju
                    #Pod zavrsavanjem partije radimo nekoliko stvari
                    #Enablujemo buttona za sledecu partiju i izlaz iz igre
                    #Upisujemo informacije u file
                    #Resetujemo board tako da sva polja budu 10
                    #Ako nema pobednika, prelazimo na sledeceg igraca i nastavljamo partiju
                    if gameStatus == 1:
                        player = 1
                        print("Pobedio je prvi igrac")
                        ex.buttonNext.setEnabled(True)
                        ex.buttonStop.setEnabled(True)
                        arduino.write('1'.encode())
                        writeResultsToFile()
                        board = np.full((3,3),10)
                
                 else:
                    sendCurrentPlayerToArduino(player)
                    while arduinoData == 3:
                        pass
                    x2 = arduinoData
                    arduinoData = 3
                    while arduinoData == 3:
                        pass
                    y2 = arduinoData
                    arduinoData = 3
                
                    if checkIfClaimed(y2, x2, board) == 1:
                        print("Izabrano polje je vec zauzeto, unesite ponovo")
                    else:
                        board[y2, x2] = 0
                        player = 1
                        print(board)
                        gameStatus = checkForWinner(board)
                    
                    if gameStatus == 0:
                        player = 1
                        print("Pobedio je drugi igrac")
                        ex.buttonNext.setEnabled(True)
                        ex.buttonStop.setEnabled(True)
                        arduino.write('2'.encode())
                        writeResultsToFile()
                        board = np.full((3,3),10)
    


t3 = Thread(target = igra)
t1.start()