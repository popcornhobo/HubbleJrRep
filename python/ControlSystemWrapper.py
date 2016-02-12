
import ctypes

_controlSystem = ctypes.CDLL('../control_system_v0.1/control_system_lib.so')
_controlSystem.control_system_update.argtypes = (ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double)
_controlSystem.control_system_update.restype = ctypes.c_double

def control_system_update(q0, q1, q2, q3):
	global _controlSystem
	result = _controlSystem.control_system_update(ctypes.c_double(q0), ctypes.c_double(q1), ctypes.c_double(q2), ctypes.c_double(q3))
	return float(result)