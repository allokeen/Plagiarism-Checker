#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStateMachine>
#include <QHistoryState>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QString>
#include <QDirIterator>
#include <QTableWidget>
#include <QtCore/qmath.h>
#include "simpleCompare.h"
#include "removeDuplicates.h"
#include "basicLexicalAnalyzer.h"
#include "lexicalAnalyzer.h"
//#include "../../src/mainCompare2Files.cpp"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    auto stateMachine = new QStateMachine(this);
    auto SelectFile = new QState(stateMachine);
    auto PathSelected = new QState(stateMachine);
    auto Error = new QState(stateMachine);
    auto Compare = new QState(stateMachine);
    auto ViewResult = new QState(stateMachine);
    SelectFile->assignProperty(ui->pbStart, "enabled", false);
    SelectFile->assignProperty(ui->pbSelect, "enabled", true);
    SelectFile->assignProperty(ui->tePath, "enabled", true);
    SelectFile->assignProperty(ui->tePath, "placeholderText", "No file selected...");
    SelectFile->assignProperty(ui->cbAlgo, "checked", true);
    connect(ui->pbSelect, SIGNAL(clicked()), this, SLOT(dialog()));
    SelectFile->addTransition(ui->tePath, SIGNAL(textChanged()), PathSelected);
    PathSelected->assignProperty(ui->pbStart, "enabled", true);
    PathSelected->assignProperty(ui->pbStart, "text", "START");
    PathSelected->assignProperty(ui->pbSelect, "enabled", true);
    PathSelected->addTransition(ui->pbStart, SIGNAL(clicked()), Compare);
    connect(Compare, SIGNAL(entered()), this, SLOT(open()));
    Compare->assignProperty(ui->pbStart, "text", "CANCEL");
    Compare->assignProperty(ui->pbSelect, "enabled", false);
    Compare->addTransition(ui->pbStart, SIGNAL(clicked()), PathSelected);
    Compare->addTransition(this, SIGNAL(done()), ViewResult);    
    Compare->addTransition(this, SIGNAL(error()), Error);
    Error->assignProperty(ui->pbStart, "enabled", false);
    Error->assignProperty(ui->pbSelect, "enabled", true);
    Error->addTransition(ui->tePath, SIGNAL(textChanged()), PathSelected);

    connect(Compare, SIGNAL(entered()), this, SLOT( viewTable()));

    ViewResult->assignProperty(ui->pbStart, "enabled", false);
    ViewResult->assignProperty(ui->pbSelect, "enabled", true);
    ViewResult->assignProperty(ui->pbStart, "text", "DONE");
    ViewResult->addTransition(ui->tePath, SIGNAL(textChanged()), PathSelected);
    ViewResult->addTransition(ui->horizontalSlider, &QSlider::sliderReleased, PathSelected);
    ViewResult->addTransition(ui->cbAlgo, SIGNAL(clicked()), PathSelected);
    ViewResult->addTransition(ui->cbAlgo_2, SIGNAL(clicked()), PathSelected);
    ViewResult->addTransition(ui->cbAlgo_3, SIGNAL(clicked()), PathSelected);
    ViewResult->addTransition(ui->cbAlgo_4, SIGNAL(clicked()), PathSelected);

    connect(ui->tableWidget, SIGNAL(cellClicked(int ,int )), this, SLOT( diff()));
    connect(ui->tableWidget, SIGNAL(cellClicked(int ,int )), this, SLOT( slider(int, int)));

    stateMachine->setInitialState(SelectFile);

    stateMachine->start();
}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::clear()
{
    allProjects.clear();
}
void MainWindow::averageValuesForProjects()
{

    allProjectsResults.resize(allProjects.size());
    allProjectsMaxFileResults.resize(allProjects.size());
        for(unsigned long a=0;a<allProjectsResults.size(); a++)
        {
            allProjectsResults[a].resize(allProjects.size());
            allProjectsMaxFileResults[a].resize(allProjects.size());
            for(unsigned long b=0;b<allProjectsResults.size(); b++)
            {
                allProjectsMaxFileResults[a][b].resize(2);
                double max=0;
                std::string file1 = "";
                std::string file2 = "";
                for(unsigned long i=0;i<allResults[a][b].size(); i++)
                {
                    for(unsigned long j=0;j<allResults[a][b][i].size(); j++)
                    {
                        if (max<allResults[a][b][i][j]) {
                            max=allResults[a][b][i][j];
                            file1 =  allProjects[a][i];
                            file2 = allProjects[b][j];
                        }
                        allProjectsResults[a][b]=max;
                        allProjectsMaxFileResults[a][b][0]=file1;
                        allProjectsMaxFileResults[a][b][1]=file2;
                        max = 0;
                        file1="";
                        file2="";
                    }
                }
            }
        }
}
void MainWindow::viewTable()
{
    averageValuesForProjects();
    int numberOfProjects=ProjectNames.size() & INT_MAX;
    //qDebug() << numberOfProjects;
    QStringList stringlist;
    QString *projectsNames = new QString[numberOfProjects];

    wyniki = new double[allProjectsResults.size()*allProjectsResults.size()];
    int l=0;
    for(unsigned long a=0;a<allProjectsResults.size(); a++)
        for(unsigned long b=0;b<allProjectsResults.size(); b++){
            //qDebug() << wyniki[l];
            wyniki[l]=allProjectsResults[a][b];
            //qDebug() << "All projects a/b from viewTable" << allProjectsResults[a][b];
            l++;
        }
    for( unsigned long i=0; i<ProjectNames.size(); i++)               //nazwy katalogów projektów
    {
        stringlist << QString::fromStdString(ProjectNames[i]);
        projectsNames[i] = QString::fromStdString(ProjectNames[i]);
    }
    ui->tableWidget->setRowCount(numberOfProjects);
    ui->tableWidget->setColumnCount(numberOfProjects);
    int k=0;
        for(int i=0; i<numberOfProjects; i++)
        {
            ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(projectsNames[i]));
            ui->tableWidget->setVerticalHeaderItem(i, new QTableWidgetItem(projectsNames[i]));
            for( int j=0; j<numberOfProjects; j++)
            {
                double wartosc = allProjectsResults[i][j];
                QString valueAsString = QString::number(wartosc);
                ui->tableWidget->setItem(i,j, new QTableWidgetItem(valueAsString));
                //COLORS
                int red = 255;
                int green = 255;
                double color = 255/50;    //wspolczynnik koloru dla jednego punktu procentowego
                if( wartosc > 50.0)
                    green = qFloor((100-wartosc)*color);
                else if( wartosc < 50.0)
                    red = qFloor(wartosc*color);
                // test for color palette
                red=255;
                green=255;
                if( wartosc > 50.0)
                {
                    green=(100-wartosc)*2*255/100;
                }
                else if( wartosc < 50.0)
                {
                    red=wartosc*2*255/100;
                }
                if(i!=j)
                    ui->tableWidget->item(i,j)->setBackgroundColor(QColor(red, green , 0, 127));
                else{
                    ui->tableWidget->item(i,j)->setBackgroundColor(QColor(255, 255 , 255, 127));
                }
                k++;
            }
        }
        std::cout << "\n+++++++++++++++++++++++++++\n";
        for(unsigned int i=0; i< allProjectsMaxFileResults.size(); i++)
             for(unsigned int j=0; j< allProjectsMaxFileResults.size(); j++)
             {
                        std::cout << "PLIKI MAKSIMUM DLA PROJEKTU " << i << " oraz " << j << "\n";
                       std::cout << allProjectsMaxFileResults[i][j][0] << "\n";
                       std::cout << allProjectsMaxFileResults[i][j][1] << "\n";
             }
        std::cout << "\n+++++++++++++++++++++++++++\n";
}

void MainWindow::diff()
{
    rowNum= ui->tableWidget->selectionModel()->currentIndex().row();
    colNum= ui->tableWidget->selectionModel()->currentIndex().column();

    std::string l1 = ProjectNames[rowNum];
    std::string l2 = ProjectNames[colNum];
    emit projectLabels(l1,l2);

    std::string path1 = allProjectsMaxFileResults[rowNum][colNum][0];
    std::string path2 = allProjectsMaxFileResults[rowNum][colNum][1];

    emit filesPath(path1, path2);
}

void MainWindow::slider(int, int)
{
    rowNum= ui->tableWidget->selectionModel()->currentIndex().row();
    colNum= ui->tableWidget->selectionModel()->currentIndex().column();

    ui->horizontalSlider->setValue(allProjectsResults[rowNum][colNum]);
    ui->horizontalSlider->valueChanged(allProjectsResults[rowNum][colNum]);
}
void MainWindow::dialog()
{
    this->directoryName = QFileDialog::getExistingDirectory(this, tr("Open file"), "..");
    ui->tePath->setPlainText(directoryName);
}
void MainWindow::open()
{
    QDir dir(ui->tePath->toPlainText());
    if (dir.exists()==0)
    {
        QMessageBox::information(this, tr("Error"), "Directory doesn't exist.");
        emit error();
        return;
    }
    QDirIterator projectIter(ui->tePath->toPlainText(), QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);

    while(projectIter.hasNext())
    {
        QDirIterator fileIter(projectIter.next(), QDir::Files | QDir::Readable, QDirIterator::Subdirectories);
        //std::vector <std::string> allPaths;
        while(fileIter.hasNext())
        {
            QFile file(fileIter.next());
            qDebug() << fileIter.filePath();
            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox::information(this, tr("Error"), file.errorString());
                emit error();
                return;
            }
            else {
                allPaths.push_back(fileIter.filePath().toStdString());
                file.close();
            }
        }
        allProjects.push_back(allPaths);
        qDebug() << projectIter.filePath() << " NAZWA PROJEKTU\n";
        std::string s = projectIter.filePath().toStdString();
        std::string delimiter = "/";
        size_t pos = 0;
        std::string token;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            s.erase(0, pos + delimiter.length());
        }
        ProjectNames.push_back(s);
    }
    if(ui->cbAlgo->checkState())
    {
        algorithmsUsed|=0x01;
        numberOfAlgorithmsUsed++;
    }
    if(ui->cbAlgo_2->checkState())
    {
        algorithmsUsed|=0x02;
        numberOfAlgorithmsUsed++;
    }
    if(ui->cbAlgo_3->checkState())
    {
        algorithmsUsed|=0x04;
        numberOfAlgorithmsUsed++;
    }
    if(ui->cbAlgo_4->checkState())
    {
        algorithmsUsed|=0x08;
        numberOfAlgorithmsUsed++;
    }
    for(unsigned long a=0;a<allProjects.size(); a++)
    {
        allResults.resize(a+1);
        for(unsigned long b=0;b<allProjects.size(); b++)
        {
            allResults[a].resize(b+1);
            if(a<b) // round-robin for projects
            {
                for(unsigned long i=0;i<allProjects[a].size(); i++)
                {
                    allResults[a][b].resize(i+1);
                    for(unsigned long j=0;j<allProjects[b].size(); j++)
                    {
                        allResults[a][b][i].resize(j+1);
                        std::string firstExtension = getFileExtension(allProjects[a][i]);
                        std::string secondExtension = getFileExtension(allProjects[b][j]);
                        //prevent compare of the same projects
                        if( (allProjects[a][i].find(ProjectNames[a]) != std::string::npos) && (allProjects[b][j].find(ProjectNames[b]) != std::string::npos)){
                            if( firstExtension == secondExtension ){
                                allResults[a][b][i][j] = compare(allProjects[a][i], allProjects[b][j], algorithmsUsed); // result of comparing file i with file j, in projects a and project b
                                qDebug() << QString::fromStdString(allProjects[a][i]) << "\n" << QString::fromStdString(allProjects[b][j]);
                                qDebug() << QString("%1").arg(allResults[a][b][i][j]) << "\n";
                            }
                        }
                    }
                }
            }
            else if(a>b)
            {
                allResults[a][b].resize(allProjects[a].size());
                for(unsigned long i=0;i<allProjects[a].size(); i++)
                {
                    allResults[a][b][i].resize(allProjects[b].size());
                    for(unsigned long j=0;j<allProjects[b].size(); j++)
                    {
                        //qDebug() << QString::fromStdString(allProjects[a][i]) << "\n" << QString::fromStdString(allProjects[b][j]);
                        allResults[a][b][i][j] = allResults[b][a][j][i];    // cutting number of compare() usages by half
                        //qDebug() << i << j << ":" << QString("%1").arg(allResults[a][b][i][j]) << "\n";
                    }
                }
            }
        }
    }
    algorithmsUsed=0;
    numberOfAlgorithmsUsed=0;
    QMessageBox::information(this, tr("Success"), QString("Directory compared to database."));
    emit done();
}
double MainWindow::compare(std::string file1, std::string file2, int algorithmsUsed)
{
    //qDebug() << "Inside comparing function... please implement.";
    //qDebug() << "algos used code: " << algorithmsUsed;
    /* Algorytmy, które masz użyć są zapisane binarnie w zmiennej algorithmsUsed
        wartość 1 to algorytm 1. wartość 2 to algo 2, 3 to algorytmy 1 i 2 itd. */
    simpleCompare s{};
    removeDuplicates r{};
    basicLexicalAnalyzer b{};
    lexicalAnalyzer l{};
    std::vector<float> res;
    switch(algorithmsUsed)
    {
        case 1:
        {
            res.push_back( s.compare(file1, file2) );
            break;
        }
        case 2:
        {
            res.push_back(r.compare(file1, file2));
            break;
        }
        case 3:
        {
            res.push_back(s.compare(file1, file2));
            res.push_back(r.compare(file1, file2));
            break;
        }
        case 4:
        {
            res.push_back(b.compare(file1, file2));
            break;
        }
        case 5:
        {
            res.push_back(s.compare(file1, file2));
            res.push_back(b.compare(file1, file2));
            break;
        }
        case 6:
        {
            res.push_back(r.compare(file1, file2));
            res.push_back(b.compare(file1, file2));
            break;
        }
        case 7:
        {
            res.push_back(s.compare(file1, file2));
            res.push_back(r.compare(file1, file2));
            res.push_back(b.compare(file1, file2));
            break;
        }
        case 8:
        {
            res.push_back(l.compare(file1, file2));
            break;
        }
        case 9:
        {
            res.push_back(s.compare(file1, file2));
            res.push_back(l.compare(file1, file2));
            break;
        }
        case 10:
        {
            res.push_back(r.compare(file1, file2));
            res.push_back(l.compare(file1, file2));
            break;
        }
        case 11:
        {
            res.push_back(s.compare(file1, file2));
            res.push_back(r.compare(file1, file2));
            res.push_back(l.compare(file1, file2));
            break;
        }
        case 12:
        {
            res.push_back(l.compare(file1, file2));
            res.push_back(b.compare(file1, file2));
            break;
        }
        case 13:
        {
            res.push_back(s.compare(file1, file2));
            res.push_back(l.compare(file1, file2));
            res.push_back(b.compare(file1, file2));
            break;
        }
        case 14:
        {
            res.push_back(r.compare(file1, file2));
            res.push_back(l.compare(file1, file2));
            res.push_back(b.compare(file1, file2));
            break;
        }
        case 15:
        {
            res.push_back(s.compare(file1, file2));
            res.push_back(r.compare(file1, file2));
            res.push_back(l.compare(file1, file2));
            res.push_back(b.compare(file1, file2));
            break;
        }
    }
    // 1. WYBRAĆ WARTOŚĆ Z TABELI WYNIKÓW PORÓWNANIA DWÓCH PLIKÓW
    float result = 0;
    for(const auto &i : res){
        if(std::isnan(i)){
            break;
        }
        else{
            result += i;
        }
    }
    res.clear();
    // 2. ZWRÓCIĆ ŚREDNIĄ TYCH WYNIKÓW DO RETURNA, zmień randa na twój wynik procentowy
    return result/numberOfAlgorithmsUsed;
}
std::string MainWindow::getFileExtension(const std::string &s)
{
    size_t i = s.rfind('.', s.length());
    if( i != std::string::npos ){
        return(s.substr(i+1, s.length() - i) );
    }
    return("");
}
