--A = color (translate (sphere (10),-3,0,0),1,0,0)
--B = color (translate (sphere (10),3,0,0),0,1,0)
--C = color (translate (sphere (10),0,3,0),0,0,1)
A = sphere (10):translate (-3,0,0):color (1,0,0)
B = sphere (10):translate (3,0,0):color (0,1,0)
C = sphere (10):translate (0,3,0):color (0,0,1)

return A*B-C
