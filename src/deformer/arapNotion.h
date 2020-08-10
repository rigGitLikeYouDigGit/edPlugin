



/*
arap deformation algorithm notes

arap works by defining "cells" on mesh, and minimising their local deformation
over the course of the global deformation

for mesh S, and deformed mesh S':
  each cell is defined as vertex V and its ring - experiment with wider
  corresponding cells C on S and S' are compared

  best-fitting rotation matrix between C and C' by minimising energy(C, C'):
    for j in ring(i):
      Sum( [ weight(i, j) * length( (i - j) - R(i' - j')) ] )

    weight(i, j) is a per-edge weight


cotangent weights for edge(i, j):
  Wcotan(i, j) = 0.5 * ( cotan(A) + cotan(B) )
  where A, B are angles on either side of edge
  this in theory is robust to mesh irregularities

  cotan(vecA, vecB){
    (A.x * B.x + A.y * B.y) / (A.x * B.y - A.y * B.x) ?
    fallback a.dot(b)) / (a.cross(b)).norm()
  }

cotαi j +cotβi






*/
