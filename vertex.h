#ifndef VERTEX_H
#define VERTEX_H

#include <QDateTime>

class Vertex
{
public:
    Vertex(QPair<double,double> geo, QDateTime timestamp);
    Vertex(QPair<double,double> geo);

    Vertex * add_adjacent(Vertex * v);
    void remove_adjacent(Vertex * v);
    void clear_adjacent();
    QList<Vertex*> getAdj();

    bool is_absorbed();
    void set_absorbed(bool val);

    double getSumEdgeLength() const;

    double getLat() const;
    double getLon() const;
    void reposition(Vertex * v);
    int getDegree() const;


    QDateTime getT() const;

    void absorbs(Vertex * v);
    QList<Vertex*> get_abs_list();

    int getWeight() const;
    void setWeight(int w);


    void setIndex(int i);
    int getIndex() const;

private:
    QList<Vertex*> adjacent_list;
    QList<Vertex*> absorbed_list;
    QDateTime t;
    bool absorbed;
    QPair<double,double> geo;
    int weight;
    int index;
};

#endif // VERTEX_H
