
import ctypes

_controlSystem = ctypes.CDLL('control_system.so')

_controlSystem.control_system_update.restype = ctypes.c_int
_controlSystem.control_system_update.argtypes = (ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float))

_controlSystem.update_gains.argtypes = (ctypes.c_float, ctypes.c_float, ctypes.c_float)

_controlSystem.rotate_current_position.argtypes = (ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float)

def control_system_update():
	global _controlSystem
	errx = ctypes.c_float		# Initialize three variable for the error values with the correct type
	erry = ctypes.c_float
	errz = ctypes.c_float
	result = _controlSystem.control_system_update(ctypes.byref(errx), ctypes.byref(erry), ctypes.byref(errz))		# Pass the error vars by ref
	return [int(result),errx,erry,errz]

def set_as_current_position():
	global _controlSystem
	_controlSystem.set_as_current_position()

def rotate_current_position(yaw, pitch, roll):
	global _controlSystem
	_controlSystem.rotate_current_position(yaw,pitch,roll)

def update_gains(p, i, d):
	global _controlSystem
	_controlSystem.update_gains(p,i,d)