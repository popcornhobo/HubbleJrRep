
import ctypes

_controlSystem = ctypes.CDLL('control_system.so')

_controlSystem.control_system_update.restype = ctypes.c_int
#_controlSystem.control_system_update.argtypes = (ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float))

_controlSystem.update_gains.argtypes = (ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float))

_controlSystem.rotate_current_position.argtypes = (ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float)

def control_system_update():
	global _controlSystem
	#errx = ctypes.LP_c_float																			# Initialize three variable for the error values with the correct type
	#erry = ctypes.LP_c_float	
	#errz = ctypes.LP_c_float	
	result = _controlSystem.control_system_update()		# Pass the error vars by ref
	return int(result)

def set_as_current_position():
	global _controlSystem
	_controlSystem.set_as_current_position()

def rotate_current_position(yaw, pitch, roll):
	global _controlSystem
	_controlSystem.rotate_current_position(yaw,pitch,roll)

def update_gains(p, i, d):
	global _controlSystem
	arraytpe = ctypes.c_float *3;
	_controlSystem.update_gains(arraytpe(p),arraytpe(i),arraytpe(d))