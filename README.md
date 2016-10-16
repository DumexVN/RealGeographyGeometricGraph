# RealGeographyGeometricGraph

Real metadata geographical/Geometric Graph Builder.

Given metadata in form of geometric data i.e. a point consists of latitude and longitude coordinates, parse (a simple JSON parser is included) and build a geometric graph G i.e. for every pair of vertex, an edge is added if their distance (Haversine) is $\leq d$ for some predefined d. 

For visual presentation, G can be plotted on world map in QGIS using 'PhotoMapper' (see other repo).

Built with qmake with qt 5.2 and BGL 1.5. 
