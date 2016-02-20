
import ctypes

_controlSystem = ctypes.CDLL('control_system.so')

_controlSystem.control_system_update.restype = ctypes.c_int

_controlSystem.update_gains.argtypes = (ctypes.c_float, ctypes.c_float, ctypes.c_float)

_controlSystem.rotate_current_position.argtypes = (ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float)

def control_system_update():
	global _controlSystem
	result = _controlSystem.control_system_update()
	return int(result)

def set_as_current_position():
	global _controlSystem
	print "help me"
	_controlSystem.set_as_current_position()
	print "im saved"

def rotate_current_position(yaw, pitch, roll):
	global _controlSystem
	_controlSystem.rotate_current_position(yaw,pitch,roll)

def update_gains(p, i, d):
	global _controlSystem
	_controlSystem.update_gains(p,i,d)