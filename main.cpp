#include "vertex.h"

#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <limits>
#include <QFile>
#include <QTextStream>

//QJSON
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <iostream>
#include <random>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>


QList<Vertex*> vertexList;
QList<QPair<int,int> > edgeList;
std::random_device rd;
std::mt19937 re(rd());
double DIST_LIMIT = 0.05;
double traverse_distance = 0.4;
double speed_limit = 4.0;
double t = 30.0;
bool aggregated = false;


// ----------------------------- HAVERSINE -----------------------------------------------------
double calculate_degree_from_decimal(double x)
{
    const double Pi = 3.141592653589793;
    double Degrees = x * Pi / 180;
    return Degrees;
}


double calculate_distance_Harvensine(Vertex * v1, Vertex * v2)
{
    double lat1 = v1->getLat(), lon1 = v1->getLon(),  lat2 = v2->getLat(), lon2 = v2->getLon();
    const double R = 6371.0;

    double dlon = calculate_degree_from_decimal(lon2 - lon1);
    double dlat = calculate_degree_from_decimal(lat2 - lat1);
    double deglat1 = calculate_degree_from_decimal(lat1);
    double deglat2 = calculate_degree_from_decimal(lat2);

    double a = sin(dlat/2)*sin(dlat/2) + cos(deglat1) * cos(deglat2) * sin(dlon/2)*sin(dlon/2);
    double c = 2 * atan2( sqrt(a), sqrt(1-a) );
    double d = R * c;

    return d;
}

// ------------------------------------- READ JSON IN -------------------------------------------
/** PARSE JSON FOLDERS CONTAINS ALL GPS COORDINATES
 * @brief read_json_files
 * @param dirPath
 * @param owner
 */
void read_json_files(QString dirPath) //reading json files for Vertexs
{   // READ DIR AND FILTERS
    QDir dir(dirPath);
    if (!dir.exists())
    {
        qWarning("Cannot find the example directory");
        return;
    }
    QStringList filters;
    filters << "*.txt";
    dir.setNameFilters(filters);
    QFileInfoList fileList = dir.entryInfoList();
    //read each json file and create users/Vertexs accordingly

    for (int i = 0; i < fileList.size(); i++)
    {
        QString filePath = fileList.at(i).filePath();
        QFile file(filePath);
        if (!file.exists()) // throw
            qWarning("Cannot find the given file");
        file.open(QFile::ReadOnly | QFile::Text);
        //begin reading JSON
        QString str = file.readAll(); // parsing
        QJsonDocument document = QJsonDocument::fromJson(str.toUtf8());
        QJsonObject jsonObj = document.object();
        QJsonObject photoArray = jsonObj["photos"].toObject();
        QJsonArray photosArray = photoArray["photo"].toArray();
        //extracting individual Vertexs
        for (int i = 0; i < photosArray.size(); i++)
        {
            //extracting metadata: CURRENT: ID ** LATITUDE ** LONGITUDE ** TAGS ** DATETIME
            QJsonObject photo = photosArray.at(i).toObject();
            QString id = photo["id"].toString();
            double lat = 0, longi = 0;
            int accuracy = 0;
            if (photo["latitude"].type() == 2){
                lat = photo["latitude"].toDouble();
                longi = photo["longitude"].toDouble();
                accuracy = photo["accuracy"].toInt();
            }
            else if (photo["latitude"].type() == 3)
            {
                lat = photo["latitude"].toString().toDouble();
                longi = photo["longitude"].toString().toDouble();
                accuracy = photo["accuracy"].toString().toInt();
            }
            QPair<double,double> latlon = qMakePair(lat,longi);
            QStringList tags = photo["tags"].toString().split(" ");
            QString datetime_string = photo["datetaken"].toString();
            QDateTime datetime = QDateTime::fromString(datetime_string, "yyyy-MM-dd hh:mm:ss");

            if (lat == 0 && longi == 0){
            }
            else
            {
                Vertex * v = new Vertex(latlon);
                vertexList.append(v);
            }
        }
    }
}

/** PARSE SINGLE FOLDER/USER IN THE DIRECTORY
 * @brief read_input
 * @param filePath
 */
void read_input(QString filePath) // READ ROOT DIRECTORY FOR ALL INDIVIDUALS VertexS
{ // GET INPUT DIR FROM DIALOG
    QDir dir(filePath);
    if (!dir.exists())
    {
        qWarning("Cannot find RootDir or RootDir not exist");
        return;
    }
    // LIST ALL DIR IN CURRENT PATH
    QFileInfoList dirList = dir.entryInfoList();

    // START LOOPING
    /*
     * TEST RUN ON ONLY 98 USERS
     */
    int no_of_input = dirList.size();
    for (int i = 2; i < no_of_input; i++) //2 first files are system files
    {

        QFileInfo dirInfo = dirList.at(i);
        if (!dirInfo.isDir())
            qWarning("Found one is not Dir");
        else
        {
            std::cout << "Loading User Vertex: " << i << std::endl;
            QString dirPath = dirInfo.filePath();
            QString userName = dirInfo.fileName().remove("TokyoRound1UserVertex");
            read_json_files(dirPath);
        }

    }
    std::cout << "TOTAL FILES: " << vertexList.size();
}

/** SORT THE LOADED VERTICES INTO A SMALLER REGION
 * @brief sortVertexIntoBoundingRect
 */
void sortVertexIntoBoundingRect()
{
    /* BIG REGION
    West-Most: -0.5286,51.5073
    North-Most: -0.11184,51.69823
    South-Most: -0.0801,51.2857
    East-Most: 0.3164,51.5662
    -> TOPLEFT: -0.5286, 51.69823
    -> BOTTOMRIGHT: -0.0801, 51.5662
    */

    /* SMALLER
        /*Current Bounding RECT: WESTMINSTER
         * TOPLEFT:51.501962, -0.126251
         * BOTTOMLEFT:51.499839, -0.126251
         * Bottom-Right: 51.499839, -0.122408
     */


    std::cout << "SORTING VertexS INTO LONDON BOUNDING RECT ... " << std::endl;
    QPair<double,double> topleft = qMakePair(-0.13281, 51.51020);
    QPair<double,double> topright = qMakePair(-0.122408, 51.501962);
    QPair<double,double> bottomright = qMakePair(-0.11221, 51.50093);
    QPair<double,double> bottomleft = qMakePair(-0.124536, 51.499839);
    QMutableListIterator<Vertex*> i(vertexList);
    while (i.hasNext())
    {
        i.next();
        Vertex * v = i.value();
        double lat = v->getLat();
        double lon = v->getLon();

        if ( (lon > topleft.first && lon < bottomright.first) &&
             (lat > bottomright.second && lat < topleft.second))
        {
           //true
        }
        /*
        if ( (lon > topright.first && lon < bottomleft.first) &&
             (lat > bottomleft.second && lat < topright.second))
        {
           //true
        }
        */
        else
        {
            i.remove();
            delete v;
        }
    }
   std::cout << "FINISHED SORTING VertexS INTO LONDON REGION! **Vertex COUNTS: " << vertexList.size() << std::endl;
}

/** Adding Edges between Vertices
 * @brief connectGraphInProximity
 * @param D: Distance Limit (Using Haversine formula 1 = 1km)
 */
void connectGraphInProximity(const double D)
{
    QList<QPair<int,int> > edges;
    for (int i = 0 ; i < vertexList.size(); i++)
    {
        Vertex * v = vertexList.at(i);
        for (int j = i + 1; j < vertexList.size(); j++)
        {
            Vertex * v2 = vertexList.at(j);
            double dist = calculate_distance_Harvensine(v,v2);
            if (dist <= D)
            {
                QPair<int,int> edge = qMakePair(i, j), copy_edge = qMakePair(j, i);
                if (!(edges.contains(edge) || edges.contains(copy_edge)))
                {
                    edges.append(edge);
                    edges.append(copy_edge);
                }
            }
        }
        qDebug () << "Done: V" << i;
    }

    for (int i = 0; i < edges.size(); i+=2)
    {
        QPair<int,int> e = edges.at(i);
        edgeList.append(e);
    }
}

/** Reset the graph
 * @brief resetForNewGraph
 */
void resetForNewGraph()
{
    edgeList.clear(); //for now
}

/** I/O LOAD GRAPH FROM FILE (FASTER THAN PARSING JSON)
 * @brief LoadGraphVertex
 */
void LoadGraphVertex()
{
    QString filePath = "C:/Users/Dumex/Desktop/TestGraph/vertex.txt";
    QFile file(filePath);
    if (!file.exists())
        qWarning("FILE NOT FOUND");
    else
    {
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QStringList str = in.readLine().split(";");
            if (str.startsWith(QString("lat")))
                continue;
            QPair<double,double> geo = qMakePair(str[0].toDouble(), str[1].toDouble());
            Vertex * v = new Vertex(geo);
            vertexList.append(v);
        }
    }
    qDebug() << "Finished Loading! Number of Vertices: " << vertexList.size();
}


/** I/O SAVING CURRENT GRAPH
 * @brief saveGraphOutput
 */
void saveVertexOutput()
{
    QString filePath = "C:/Users/Dumex/Desktop/TestGraph/vertex.txt";
    QFile file(filePath);
    file.open(QFile::WriteOnly | QFile::Text);
    QTextStream out(&file);
    out << "lat;long;id" << endl;
    out.setRealNumberPrecision(8);
    for ( int i = 0 ; i < vertexList.size() ;i++)
    {
        Vertex * v = vertexList.at(i);
        out << v->getLat() <<";"<<v->getLon() << ";" << i <<endl;
    }
    file.close();
}

void saveEdgeOutput(const double edgeLeght)
{
    QString filePath = "C:/Users/Dumex/Desktop/TestGraph/edge_"
            + QString::number(edgeLeght)
            + "_km.txt";
    QFile file(filePath);
    file.open(QFile::WriteOnly | QFile::Text);
    QTextStream out(&file);
    out.setRealNumberPrecision(8);
    for ( int i = 0 ; i < edgeList.size() ;i++)
    {
        QPair<int,int> p =edgeList.at(i);
        out << p.first <<"," << p.second <<endl;
    }
    file.close();
    qDebug() << "Finished Writing: Save Edge Out Put: for D =" << edgeLeght;
}
// ----------------------------- MAIN STUFF -----------------------------------------------------
// ------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    std::cout << "STARTING ..." << std::endl;

    QString filePath = "C:/Users/Dumex/Documents/FlickrDataSets/MappingPhotosData/INDIVIDUALPHOTOS";
    std::cout << "LOADING PRESET FILEPATH ..." << std::endl;
    LoadGraphVertex();
    double D = 0.01;
    connectGraphInProximity(D);
    saveEdgeOutput(D);

    return 0;
}

