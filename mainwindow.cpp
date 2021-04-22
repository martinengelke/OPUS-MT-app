#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <memory>
#include <functional>
#include "MarianInterface.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QSaveFile>
#include <QDir>
#include <QMessageBox>
#include <QFontDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
    , models_(this)
    , network_(this)
{
    ui_->setupUi(this);

    // Initial text of the comboBox
    ui_->Models->insertItem(0, QString("Models"));

    // Hide download progress bar
    ui_->progressBar->hide();

    // Display local models
    for (auto&& item : models_.models_) {
        ui_->localModels->addItem(item.name);
    }
    // Load one if we have
    if (models_.models_.size() != 0) {
        resetTranslator(models_.models_[0].path);
    }
    // @TODO something is broken, this gets called n+1 times with every new model
    // This updates the local models and activates the newly downloaded one.
    auto updateLocalModels = [&](int index){
        ui_->localModels->addItem(models_.models_[index].name);
        on_localModels_activated(index);
        ui_->localModels->setCurrentIndex(index);
    };

    // Attach slots
    connect(&models_, &ModelManager::newModelAdded, this, updateLocalModels); // When a model is downloaded, update the UI
    connect(&models_, &ModelManager::error, this, &MainWindow::popupError); // All errors from the model class will be propagated to the GUI
    connect(&network_, &Network::error, this, &MainWindow::popupError); // All errors from the network class will be propagated to the GUI
}

MainWindow::~MainWindow() {
    delete ui_;
}

void MainWindow::on_actionTranslate_triggered()
{
    if (translator_) {
        if (ui_->inputBox->toPlainText() != QString("")) {
            ui_->localModels->setEnabled(false); // Disable changing the model while translating
            ui_->translateButton->setEnabled(false); //Disable the translate button before the translation finishes
            ui_->outputBox->setText("Translating, please wait...");
            this->repaint(); // Force update the UI before the translation starts so that it can show the previous text
            translator_->translate(ui_->inputBox->toPlainText());
        } else {
            // Empty input crashes the translator
            popupError("Write something to be translated first.");
        }
    } else {
        popupError("You need to download a translation model first. Do that with the interface on the right.");
    }
}

/**
 * @brief MainWindow::on_modelDownload_clicked fetches the available models json from the Internet.
 */
void MainWindow::on_modelDownload_clicked()
{
    QString url("http://data.statmt.org/bergamot/models/models.json");
    connect(&network_, &Network::getJson, this, &MainWindow::onResult);
    network_.downloadJson(url);
}

/**
 * @brief MainWindow::onResult reads the json for available models
 */
void MainWindow::onResult(QJsonObject obj)
{
    static bool success = false;
    if (!success) { // Success
        QJsonArray array = obj["models"].toArray();
        for (auto&& arrobj : array) {
            QString name = arrobj.toObject()["name"].toString();
            QString code = arrobj.toObject()["code"].toString();
            QString url = arrobj.toObject()["url"].toString();
            urls_.append(url);
            codes_.append(code);
            names_.append(name);
        }
        ui_->Models->removeItem(0);
        ui_->Models->insertItems(0, codes_);
        success = true;
    }
}

void MainWindow::handleDownload(QString filename, QByteArray data) {
    models_.writeModel(filename, data);
    // Re-enable model downloading interface:
    ui_->Models->setEnabled(true);
    // Hide progressBar
    ui_->progressBar->hide();
}

/**
 * @brief MainWindow::on_Models_activated Download available models or setup new ones
 * @param index
 */
void MainWindow::on_Models_activated(int index)
{
    if (names_.size() == 0) { // Fetch the models if they are not there
        on_modelDownload_clicked();
    } else { // If models are fetched download the selected model
        //@TODO check if model is already downloaded and prompt user for download confirmation
        connect(&network_, &Network::progressBar, this, &MainWindow::downloadProgress, Qt::UniqueConnection);
        connect(&network_, &Network::downloadComplete, this, &MainWindow::handleDownload, Qt::UniqueConnection);
        network_.downloadFile(urls_[index]);
        // Disable this section of the ui while a model is downloading..
        ui_->Models->setEnabled(false);
    }
}

void MainWindow::downloadProgress(qint64 ist, qint64 max) {
    ui_->progressBar->show();
    ui_->progressBar->setRange(0,max);
    ui_->progressBar->setValue(ist);
    if(max < 0) {
        ui_->progressBar->hide();
    }
}

/**
 * @brief MainWindow::on_localModels_activated Change the loaded translation model to something else.
 * @param index index of model in models_.models_ (model manager)
 */

void MainWindow::on_localModels_activated(int index) {
    if (models_.models_.size() > 0) {
        resetTranslator(models_.models_[index].path);
    }
}

/**
 * @brief MainWindow::resetTranslator Deletes the old translator object and creates a new one with the new language
 * @param dirname directory where the model is found
 */

void MainWindow::resetTranslator(QString dirname) {
    // Disconnect existing slots:
    if (translator_) {
        disconnect(translator_.get());
    }
    QString model0_path = dirname + "/";
    ui_->localModels->setEnabled(false); // Disable changing the model while changing the model
    ui_->actionTranslate->setEnabled(false); //Disable the translate button before the swap

    translator_.reset(); // Do this first to free the object.
    translator_.reset(new MarianInterface(model0_path, this));

    ui_->actionTranslate->setEnabled(true); // Reenable it
    ui_->localModels->setEnabled(true); // Disable changing the model while changing the model

    // Set up the connection to the translator
    connect(translator_.get(), &MarianInterface::translationReady, this, [&](QString translation){ui_->outputBox->setText(translation);
                                                                                                  ui_->localModels->setEnabled(true); // Re-enable model changing
                                                                                                  ui_->translateButton->setEnabled(true); // Re-enable button after translation is done
                                                                                                 });
}

/**
 * @brief MainWindow::popupError this will produce an error message from various subclasses
 * @param error the error message
 * NOTES: This message bug will occasionally trigger the harmless but annoying 4 year old bug https://bugreports.qt.io/browse/QTBUG-56893
 * This is basically some harmless console noise of the type: qt.qpa.xcb: QXcbConnection: XCB error: 3 (BadWindow
 */

void MainWindow::popupError(QString error) {
    QMessageBox msgBox(this);
    msgBox.setText(error);
    msgBox.exec();
}

void MainWindow::on_FontButton_clicked()
{
    this->setFont(QFontDialog::getFont(0, this->font()));
}
