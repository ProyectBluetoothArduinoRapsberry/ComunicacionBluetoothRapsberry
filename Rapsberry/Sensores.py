#!/usr/bin/python
# El programa requiere instalar la libreria pybluez para el modulo bluetooth
import thread
import time
import bluetooth
import atexit
import time
import select
import os
import sys
import string
import re
import signal
import Tkinter
import tkMessageBox
import websocket

NOMBRE = "Sensores"				# Nombre que aparecen en la ventana
ALTO = "400"					# Numero de pixeles que tendra de alto la ventana
ANCHO = "600"					# Numero de pixeles que tendra de ancho la ventana

SEPARADOR = '|'					# Simbolo que permite identificar entre los datos que llegan de la arduino donde empieza y termina cada linea
LIMIT = ","						# Simbolo que separa los datos que vienen en una linea
CANT_LINEAS = 5					# Cantidad de lineas que se deben acumular para poder guardar en el archivo de datos (si se guarda de una en una es mas lento el programa)
CANT_MENSAJES = 15				# Cantidad de mensaje que se guardaran en el historial de la aplicacion
RUTA = ""						# Carpeta donde se guardara el archivo, dejar "" para guardar en la misma carpeta del programa
CARPETA_DATOS = "DatosArduino"	# Nombre de la carpeta que guardara los datos
LIMPIAR = set(string.printable)	# Se usa para quitar los caracteres que llegan con error de la arduino
BOTONES = {	"Bomba": 'a', 	# ["Nombre funcion"] : 'numero', el nombre de funcion puede ser cualquiera pero el numero debe coincidir con el numero del comando en arduino
			"Valvula1": 'b',
			"Valvula2": 'c',
			"Valvula3": 'd',
			"Manual": 'e'}
port = 1
address = "98:D3:32:30:D6:ED"			# Id del bluetooth del arduino

idConexion = "33175355"					# Id de la conexion 

leido = ""  							# Variable que va guardando todo lo que se recibe de la arduino
anteriorFecha = ""						# Variable que se usa para verificar la fecha de los archivos de los datos y poder separar los datos de diferentes fechas en diferentes archivos

historial = []							# Historial de mensajes de la aplicacion
lineas = []								# Variable que va acumulando las lineas que se guardan en el archivo de datos
arduino = None							# Se debe iniciar desde aca la variable que se asocia con la conexion bluetooth de arduino
ventana = None							# Variable que contendra toda la funcionalidad de la ventana

mensaje = None							# Variable que tendra los mensajes que se mostraran en la aplicacion
mensajeVentana = None					# Variable que contendra el texto de los mensajes de la variable anterior
distancia = None						# Variable que tendra la distancia que se mostrara en la aplicacion
distanciaVentana = None					# Variable que contendra el texto de la variable anterior
duracion = None							# Variable que tendra la duracion que se mostraran en la aplicacion
duracionVentana = None					# Variable que contendra el texto de la variable anterior

ws = None 								# Variable que permitira la conexion y envio de informacion al servidor
botones = {}							#Definicion de los botones de la ventana, se usa para cambiar la imagen cuando pasa de on-off off-on
imageOn = None							#Definicion de la imagen usada en el boton para mostrar que esta prendido
imageOff = None							 #Definicion de la imagen usada en el boton para mostrar que esta apagado
# Guarda los estados de los sensores y la informacion de la distancia y duracion
estados = 	{ 	"Distancia" : 0,
				"Duracion" : 0,
				"Trig" : "OFF",
				"Eco" : "OFF",
				"Bomba" : "OFF",
				"Valvula1" : "OFF",
				"Valvula2" : "OFF",
				"Valvula3" : "OFF",
				"Manual" : "OFF"}   # Estados OFF->ON
				
botonServidor = None

def imprimir(texto):				# Funcion para mostrar los mensajes de la aplicacion
	global mensaje
	global mensajeVentana
	print texto
	if mensaje != None and mensajeVentana != None:		# Verifica que ya se haya creado la ventana
		if(len(historial) > 0):
			if(historial[0] != texto):						# verifica que el ultimo comando no sea el mismo
				historial.insert(0, texto)						# Inserta el mensaje en el historial
		else:
			historial.insert(0, texto)
			
		if len(historial) > CANT_MENSAJES:				# Si el historial llego a la cantidad definida, se elimina el mensaje mas antiguo
			historial.pop()								# Elimina el mensaje mas antiguo
		mensaje.set('\n'.join(historial))				# Las siguientes 2 lineas establecen el historial de mensajes en la ventana
		mensajeVentana.grid(row=2, column=1, rowspan=CANT_MENSAJES, padx=50, pady=20)


def writeFile(linea):		# Funcion que va acumulando los datos que se reciben de la Arduino y guarda esos datos en el archivo correspondiente
	global lineas			# La palabra global quiere decir que esa variable esta definida por fuera de la funcion
	global anteriorFecha
	hora = time.strftime("%H:%M:%S") 	# Obtiene la hora en formato Hora:Minutos:Segundos
	year = time.strftime("%Y")  		# Obtiene el anio
	mes = time.strftime("%m")			# Obtiene el mes
	dia = time.strftime("%d")			# Obtiene el dia
	fecha = dia + mes + year			# Une los datos para dejar la fecha en formato diaMesAnio

	if fecha != anteriorFecha or len(lineas) > CANT_LINEAS: #Entra Si la fecha cambia (cambio de dia) o si ya se acumularon los datos requeridos para guardar en el archivo
		if not os.path.exists(RUTA + CARPETA_DATOS):					#Verifica si no existe la carpeta con nombre "DatosArduino"
			os.makedirs(RUTA + CARPETA_DATOS)

		if not os.path.exists(RUTA + CARPETA_DATOS + "/" + year):					#Verifica si no existe la carpeta con ese con nombre de ese anio
			os.makedirs(RUTA + CARPETA_DATOS + "/" + year)							# Si no existe crea la carpeta

		if not os.path.exists(RUTA + CARPETA_DATOS + "/" + year + "/" + mes):					#Verifica si no existe la carpeta con nombre de ese mes
			os.makedirs(RUTA + CARPETA_DATOS + "/" + year + "/" + mes)

		if os.path.exists(RUTA + CARPETA_DATOS + "/" + year + "/" + mes + "/" + "datos_"+fecha+".txt"):	# Verifica si el archivo con la fecha obtenida ya existe
			tipoEscritura = 'a'							# Si existe dejara la opcion que no creara un archivo nuevo, sino que escribira los datos en el archivo que ya existe
		else:
			tipoEscritura = 'w'							# Si no existe deja la opcion para crear el archivo nuevo
			imprimir("Se creo el archivo: " + RUTA + CARPETA_DATOS + "/" + year + "/" + mes + "/" + "datos_"+fecha+".txt")

		archivo = open(RUTA + CARPETA_DATOS + "/" + year + "/" + mes + "/" + "datos_"+fecha+".txt", tipoEscritura) # Abre el archivo de acuerdo a la opcion que se definio arriba

		for line in lineas:
			archivo.write(line + "\n") # Escribe dato a dato en el archivo lo que se ha acumulado

		archivo.close()										# Se cierra el archivo
		lineas = []			#Vacia la variable que va acumulando los lineas

		if fecha != anteriorFecha:
			anteriorFecha = fecha		# Si hay cambio de dia, actualiza la variable que permite verificar

	lineas.append(fecha +LIMIT + hora + LIMIT + linea)	# Agrega fecha y hora al comienzo de la linea y despues Agrega los datos a la variable que va acumulando



def comunicacion():			# Funcion que va leyendo los datos que llegan de la arduino
	global leido
	global ws
	ready = select.select([arduino], [], [], 0.01)		# Select permite esperar datos que llegan de la Arduino sin bloquear el programa (Si no llega nada en 0.01 segundos sigue ejecutando)
	if ready[0]:										# Verifica si llego algun dato de la arduino
		leido += arduino.recv(1024)						# Como los datos a veces llegan cortados, se van acumulando todos los que llegan para despues organizarlos por lineas
		limite = leido.find(SEPARADOR)					# Busca si en lo que va acumulado esta el separador que identifica donde empiezan y termina cada linea
		if limite != -1:								# Verifica si se encontro el Separador
			rec = leido[:limite]						# Obtiene una linea de lo que va acumulado desde ultimo separador encontrado hasta el siguiente

			rec = filter(lambda x: x in LIMPIAR, rec) 	# Las Siguientes dos lineas eliminar cualquier letra que sea diferente a lo que se envio desde la arduino

			rec = rec.replace('\n', '').replace('\r','')

			if rec:										# Verificar que en rec si haya algo
				writeFile(rec)							# Envia la linea a la funcion writeFile				
				ActualizarEstados(rec)
				try:
					ws.send(rec)
				except:
					if(botonServidor.image != imageOff):
						botonServidor.configure(image=imageOff)
						botonServidor.image = imageOff


			leido = leido[limite+1:]					# Deja lista la variable leido para empezar a acumular desde el ultimo separador encontrado


	ventana.after(1, comunicacion)  			# Permite ejecutar continuamente esta funcion (comunicacion-ejecuta la funcion cada 1 milesima de segundo)

def ActualizarEstados(linea):
	global estados
	global ws
	global botonServidor
	# Cada linea viene de la siguiente forma  -distancia,duracion,estadoTrig,estadoEco,estadoBomba,estadoValvula1,estadoValvula2,estadoValvula3
	datos = linea.split(",")	# Obtiene la lista donde cada elemento se obtiene separando donde hay comas
	# Lista de control usada en para que los datos de la linea coincidan con lo que hay en los estados
	campos = ["Distancia","Duracion","Trig","Eco","Bomba","Valvula1","Valvula2","Valvula3","Manual"]
	
	if ws != None:
		servidorOn = ws.connected		
		if servidorOn:
			botonServidor.configure(image=imageOn)
			botonServidor.image = imageOn
		
		else:
			botonServidor.configure(image=imageOff)
			botonServidor.image = imageOff				
	else:
		botonServidor.configure(image=imageOff)
		botonServidor.image = imageOff				
	
	for campo in campos:
		if len(datos) > 0 :
			estado = datos.pop(0).strip()
			if(estados[campo] != estado):
				estados[campo] = estado

	if estados["Manual"] == "OFF":		# Si el Modo manual esta desactivado se deshabilitan los botones
		cambiarEstadoBotones(Tkinter.DISABLED)
	else:
		cambiarEstadoBotones(Tkinter.NORMAL)

	# Cambiar el boton de on a off o viceversa, si esta cambio en la Arduino
	for estado in BOTONES.keys():
		cambiarBoton(estado)
	# Atualizar los datos de distancia y duracionVentana
	if distancia != None and distanciaVentana != None:		# Verifica que ya se haya creado la ventana
		distancia.set("Distancia: " + estados["Distancia"])				# Las siguientes 2 lineas establecen la distancia que la arduino envian constantemente
		distanciaVentana.grid(row=0, column=1, pady=5)

	if duracion != None and duracionVentana != None:		# Verifica que ya se haya creado la ventana
		duracion.set("Duracion: " + estados["Duracion"])				# Las siguientes 2 lineas establecen la duracion que la arduino envian constantemente
		duracionVentana.grid(row=1, column=1, pady=5)

def comando(boton, comand):		# Funcion que envia el comando a la arduino
	imprimir("Se envio el comando: " + boton + ", #" + comand)	# Mostrar en consola
	estadoAnterior = estados[boton]

	if estadoAnterior == "OFF":		# Si esta apagado es porque la instruccion es para prender, en este caso mayuscula es prender "ON" y minuscula para apagar "OFF"
		comand = comand.upper()
	#OFF -> Minuscula | ON -> Mayuscula
	thread.start_new_thread( checkComando, (estadoAnterior, comand, boton, ) )
	

def checkComando(estadoAnterior, comando, boton):
	for i in xrange(3):
		if estadoAnterior == estados[boton]:
			arduino.sendall(comando)		# Envio del numero a la arduino			
			time.sleep(0.5)


def cambiarBoton(estado):			# Funcion que muestra si el boton esta prendido o apagado de acuerdo a la informacion que viene de la arduino
	global estados
	if estados[estado] == "ON":
		botones[estado].configure(image=imageOn)
		botones[estado].image = imageOn
	else:
		botones[estado].configure(image=imageOff)
		botones[estado].image = imageOff

def cambiarEstadoBotones(estado):			# Funcion para deshabilitar todos los botones excepto el boton ModoManual
	for boton in BOTONES.keys():
		if boton != "Manual":					# Se verifica que el boton ModoManual no se deshabilite, el resto se deshabilitan
			botones[boton].configure(state=estado)
			botones[boton].state = estado
			
def conectarServidor():
	thread.start_new_thread(conectar, ())
	
def conectar():
	global ws
	imprimir("Creando conexion con el servidor...")	
	try:
		ws = websocket.create_connection("ws://192.168.0.41:3000/rasp")
		imprimir("Creando conexion con el servidor. Finalizo correctamente")			
	except:
		imprimir("No se pudo conectar al servidor")


def recibirComandos():	
	global ws	
	result = None
	try:	
		if ws != None:	
			result = ws.recv()	
			if result.find(',') >= 0:
				result = result.split(',')
				boton = result[0]
				command = result[1]		
				comando(boton, command)		
	except ValueError:
		print "Perdida de conexion con el servidor: " + ValueError	
		conectar()
	thread.start_new_thread(recibirComandos, ())
	

def salir(signal=None, frame=None):				# Funcion que se ejecuta cuando se cierra el programa
	global mensaje
	global mensajeVentana
	global ws
	mensaje = None
	mensajeVentana = None
	imprimir("Cerrando conexion Bluethooth...")
	if arduino != None: 	# Verifica que la conexion con arduino si se haya hecho, si la hizo la cierra
		arduino.close()

	if ventana != None: 	# Verifica que si la ventana esta creada, si esta creada la destruye
		ventana.quit()
		
	if ws != None:
		ws.close()

	imprimir("Cerrando conexion Bluethooth: Finalizo correctamente")
	imprimir("Saliendo de la aplicacion...")
	sys.exit(0)


atexit.register(salir)							# Se registra la funcion salir. Cuando el programa se cierra, se ejecuta la funcion salir
signal.signal(signal.SIGINT, salir)

ventana = Tkinter.Tk()							# Se crea la ventana del programa
ventana.geometry(ANCHO + "x" + ALTO)   			# Establecer tamano de la ventana
ventana.title(NOMBRE)							# Establecer nombre de la ventana
arduino = bluetooth.BluetoothSocket( bluetooth.RFCOMM )	# Inicializar la conexion bluetooth

imprimir("Creando BOTONES...")
imageOn = Tkinter.PhotoImage(file="images/on.png")
imageOff = Tkinter.PhotoImage(file="images/off.png")
posicion = 1

lastItem = "Manual"
orderBoton = BOTONES.keys()
orderBoton.sort()
orderBoton.remove(lastItem)
orderBoton.insert(0, lastItem)

for boton in orderBoton:		# Creacion de los botones	
	botonTemporal = Tkinter.Button(ventana, text=boton, image=imageOff, compound="left", width=100, command = lambda a=boton, b=BOTONES[boton]: comando(a, b))
	botones[boton] = botonTemporal
	botonTemporal.grid(row=(posicion+1), column = 0)
	posicion = posicion + 1

botonServidor = Tkinter.Button(ventana, text="Servidor", image=imageOff, compound="left", width=100, command = lambda: conectarServidor())
botonServidor.grid(row=posicion+3, column = 0, padx=5, pady=18)
botonServidor.configure(state=Tkinter.DISABLED)
botonServidor.state = Tkinter.DISABLED	

imprimir("Creando BOTONES: Finalizo correctamente")

imprimir("Estableciendo conexion bluetooth...")

arduino.connect((address, port))		# Establecer los datos de conexion
arduino.setblocking(0)					# Establecer la conexion de tal manera que no bloquee el programa

imprimir("Estableciendo conexion bluetooth: Finalizo correctamente")

conectarServidor()

imprimir("Creando Ventana...")

mensaje = Tkinter.StringVar()
mensajeVentana = Tkinter.Label(ventana, textvariable=mensaje, relief=Tkinter.RAISED, bg="white", fg="gray")
mensajeVentana.grid(row=2, column=1, rowspan=CANT_MENSAJES, padx=50, pady=20)

distancia = Tkinter.StringVar()
distanciaVentana = Tkinter.Label(ventana, textvariable=distancia, relief=Tkinter.RAISED)
distanciaVentana.grid(row=1, column=1, pady=5)

duracion = Tkinter.StringVar()
duracionVentana = Tkinter.Label(ventana, textvariable=duracion, relief=Tkinter.RAISED)
duracionVentana.grid(row=3, column=1, pady=5)

thread.start_new_thread(recibirComandos, ())

ventana.after(1, comunicacion)
imprimir("Creando Ventana: Finalizo correctamente")
ventana.mainloop()			# ejecutar la ventana
