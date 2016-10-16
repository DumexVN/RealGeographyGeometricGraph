#include "vertex.h"

#include <iostream>

Vertex::Vertex(QPair<double, double> geo, QDateTime timestamp)
{
    this->geo = geo;
    this->t = timestamp;
    absorbed = false;
}

Vertex::Vertex(QPair<double, double> geo)
{
    this->geo = geo;
    t = QDateTime();
}

Vertex *Vertex::add_adjacent(Vertex *v)
{
    if (adjacent_list.contains(v)) {}
   //     qWarning("Duplicate adjacent vertex");
    else
        adjacent_list.append(v);
}

void Vertex::remove_adjacent(Vertex *v)
{
    if (adjacent_list.contains(v))
    {
        adjacent_list.removeAt(adjacent_list.indexOf(v));
    }
    else{}
  //      qWarning("Adjacent Vertex Not Found");
}

void Vertex::clear_adjacent()
{
    for (int i = 0; i < adjacent_list.size(); i++)
    {
        Vertex * v = adjacent_list.at(i);
        v->remove_adjacent(this);
    }
   adjacent_list.clear();
}

QList<Vertex *> Vertex::getAdj()
{
    return adjacent_list;
}

bool Vertex::is_absorbed()
{
    return absorbed;
}

void Vertex::set_absorbed(bool val)
{
    absorbed = false;
}


double Vertex::getLat() const
{
    return geo.first;
}

double Vertex::getLon() const
{
    return geo.second;
}

void Vertex::reposition(Vertex *v)
{
    if (v!=0)
    {
        QPair<double,double> newPos = qMakePair(v->getLat(), v->getLon());
        this->geo = newPos;
    }
}

int Vertex::getDegree() const
{
    return adjacent_list.size();
}

QDateTime Vertex::getT() const
{
    return this->t;
}

void Vertex::absorbs(Vertex *v)
{
    if (v == 0)
        return;
    absorbed_list.append(v->get_abs_list());
    absorbed_list.append(v);
    v->set_absorbed(true);
}

int Vertex::getWeight() const
{
    return weight;
}

void Vertex::setWeight(int w)
{
    weight = w;
}


void Vertex::setIndex(int i)
{
    index = i;
}

int Vertex::getIndex() const
{
    return index;
}

QList<Vertex*> Vertex::get_abs_list()
{
    return absorbed_list;
}
