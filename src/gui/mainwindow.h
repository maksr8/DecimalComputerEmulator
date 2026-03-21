#pragma once
#include <QMainWindow>
#include <QString>
#include <QCloseEvent>

namespace Ui
{
    class MainWindow;
}

class CpuStats;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    Ui::MainWindow* _ui;
    CpuStats* _stats;
    bool _hasUnsavedChanges;
	bool _hasUncompiledChanges;
    QString _currentFilePath;
    std::vector<int> _compiledBinary;
    std::unordered_map<int, std::string> _currentLabels;

    void setupMemoryTable();
    void setupConnections();
    bool promptSaveIfUnsaved();

    void onActionOpen();
    void onActionSave();
    void onActionCompile();
    void onTextChanged();
};