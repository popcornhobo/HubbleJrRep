
import ctypes

_controlSystem = ctypes.CDLL('control_system_lib.so')
_controlSystem.control_system_update.restype = ctypes.c_int
_controlSystem.set_as_current_position.restype = ctypes.c_void
_controlSystem.rotate_current_position.restype = ctypes.c_void
_controlSystem.update_gains.restype = ctypes.c_void

_controlSystem.update_gains.argtypes = (ctypes.c_float, ctypes.c_float, ctypes.c_float)
_controlSystem.rotate_current_position.argtypes = (ctypes.c_float, ctypes.c_float, ctypes.c_float)


def control_system_update():
	global _controlSystem
	result = _controlSystem.control_system_update()
	return int(result)

def set_as_current_position():
	global _controlSystem
	_controlSystem.set_as_current_position()

def rotate_current_position(yaw, pitch, roll):
	global _controlSystem
	_controlSystem.rotate_current_position(ctypes.c_float(yaw),ctypes.c_float(pitch),ctypes.c_float(roll))

def update_gains(p, i, d):
	global _controlSystem
	_controlSystem.update_gains(ctypes.c_float(p),ctypes.c_float(i),ctypes.c_float(d))