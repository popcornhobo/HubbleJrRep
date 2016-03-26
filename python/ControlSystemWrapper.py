
import ctypes

_controlSystem = ctypes.CDLL('control_system.so')

_controlSystem.control_system_update.restype = ctypes.c_int
#_controlSystem.control_system_update.argtypes = (ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float))

_controlSystem.update_gains.argtypes = (ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float)

_controlSystem.rotate_current_position.argtypes = (ctypes.c_float, ctypes.c_float, ctypes.c_float)

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

def rotate_current_position(pitch, yaw, roll):
	global _controlSystem
	_controlSystem.rotate_current_position(ctypes.c_float(pitch),ctypes.c_float(yaw),ctypes.c_float(roll))

def update_gains(p, i, d):
	global _controlSystem
	arraytpe = ctypes.c_float * 3;
	
	p_pitch = p[0]
	p_yaw = p[1]
	p_roll = p[2]
	
	i_pitch = i[0]
	i_yaw = i[1]
	i_roll = i[2]
	
	d_pitch = d[0]
	d_yaw = d[1]
	d_roll = d[2]

	_controlSystem.update_gains(p_pitch, p_yaw, p_roll, i_pitch, i_yaw, i_roll, d_pitch, d_yaw, d_roll,)

