#include "discordpage.h"
#include "log.h"
#include "mainwindow.h"
#include "virtmic.h"

#include <QApplication>
#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QNetworkReply>
#include <QTimer>
#include <QWebChannel>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>

DiscordPage::DiscordPage(QWidget *parent) : QWebEnginePage(parent) {
  setBackgroundColor(QColor("#202225"));

  connect(this, &QWebEnginePage::featurePermissionRequested, this,
          &DiscordPage::featurePermissionRequested);

  setupPermissions();

  injectFile(&DiscordPage::injectScript, "qwebchannel.js",
             ":/qtwebchannel/qwebchannel.js");

  setUrl(QUrl("https://discord.com/app"));

  setWebChannel(new QWebChannel(this));
  webChannel()->registerObject("userscript", &m_userScript);

  injectFile(&DiscordPage::injectScript, "userscript.js",
             ":/assets/userscript.js");

  injectFile(&DiscordPage::injectScript, "userscript.js",
             ":/assets/userscript.js");
  QFile vencord(":/assets/vencord/vencord.js");
  if (!vencord.open(QIODevice::ReadOnly))
    qFatal("Failed to load vencord source with error: %s",
           vencord.errorString().toLatin1().constData());
  injectScript(
      "vencord.js",
      QString("window.discordScreenaudioVencordSettings = `%1`; %2")
          .arg(m_userScript.vencordSend("VencordGetSettings", {}).toString(),
               vencord.readAll()));
  vencord.close();

  setupUserStyles();
}

void DiscordPage::setupPermissions() {
  settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
  settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
  settings()->setAttribute(QWebEngineSettings::AllowRunningInsecureContent,
                           true);
  settings()->setAttribute(
      QWebEngineSettings::AllowWindowActivationFromJavaScript, true);
  settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
  settings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture,
                           false);
  settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
  settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);
}

void DiscordPage::setupUserStyles() {
  QString file =
      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
      "/userstyles.css";
  if (QFileInfo(file).exists()) {
    qDebug(mainLog) << "Found userstyles:" << file;
    injectFile(&DiscordPage::injectStylesheet, "userstyles.js", file);
  }
}

void DiscordPage::injectScript(
    QString name, QString content,
    QWebEngineScript::InjectionPoint injectionPoint) {
  qDebug(mainLog) << "Injecting " << name;

  QWebEngineScript script;

  script.setSourceCode(content);
  script.setName(name);
  script.setWorldId(QWebEngineScript::MainWorld);
  script.setInjectionPoint(injectionPoint);
  script.setRunsOnSubFrames(false);

  scripts().insert(script);
}

void DiscordPage::injectScript(QString name, QString content) {
  injectScript(name, content, QWebEngineScript::DocumentCreation);
}

void DiscordPage::injectStylesheet(QString name, QString content) {
  auto script = QString(R"(const stylesheet = document.createElement("style");
stylesheet.type = "text/css";
stylesheet.id = "%1";
stylesheet.innerText = `%2`;
document.head.appendChild(stylesheet);
)")
                    .arg(name)
                    .arg(content);
  injectScript(name, script, QWebEngineScript::DocumentReady);
}

void DiscordPage::injectFile(void (DiscordPage::*inject)(QString, QString),
                             QString name, QString source) {
  QFile file(source);

  if (!file.open(QIODevice::ReadOnly)) {
    qFatal("Failed to load %s with error: %s", source.toLatin1().constData(),
           file.errorString().toLatin1().constData());
  } else {
    (this->*inject)(name, file.readAll());
  }
}

void DiscordPage::featurePermissionRequested(const QUrl &securityOrigin,
                                             QWebEnginePage::Feature feature) {
  // Allow every permission asked
  setFeaturePermission(securityOrigin, feature,
                       QWebEnginePage::PermissionGrantedByUser);

  if (feature == QWebEnginePage::Feature::MediaAudioCapture) {
    if (!m_userScript.isVirtmicRunning()) {
      qDebug(virtmicLog) << "Starting Virtmic with no target to make sure "
                            "Discord can find all the audio devices";
      m_userScript.startVirtmic("None");
    }
  }
}

bool DiscordPage::acceptNavigationRequest(const QUrl &url,
                                          QWebEnginePage::NavigationType type,
                                          bool isMainFrame) {
  if (type == QWebEnginePage::NavigationTypeLinkClicked) {
    QDesktopServices::openUrl(url);
    return false;
  }
  return true;
};

bool ExternalPage::acceptNavigationRequest(const QUrl &url,
                                           QWebEnginePage::NavigationType type,
                                           bool isMainFrame) {
  QDesktopServices::openUrl(url);
  deleteLater();
  return false;
}

QWebEnginePage *DiscordPage::createWindow(QWebEnginePage::WebWindowType type) {
  return new ExternalPage;
}

const QMap<QString, QString> cssAnsiColorMap = {{"black", "30"},
                                                {"red", "31"},
                                                {"green", "32"},
                                                {"yellow", "33"},
                                                {"blue", "34"},
                                                {"magenta", "35"},
                                                {"cyan", "36"},
                                                {"white", "37"},
                                                {"gray", "90"},
                                                {"bright-red", "91"},
                                                {"bright-green", "92"},
                                                {"bright-yellow", "93"},
                                                {"bright-blue", "94"},
                                                {"bright-magenta", "95"},
                                                {"bright-cyan", "96"},
                                                {"bright-white", "97"},
                                                {"orange", "38;5;208"},
                                                {"pink", "38;5;205"},
                                                {"brown", "38;5;94"},
                                                {"light-gray", "38;5;251"},
                                                {"dark-gray", "38;5;239"},
                                                {"light-red", "38;5;203"},
                                                {"light-green", "38;5;83"},
                                                {"light-yellow", "38;5;227"},
                                                {"light-blue", "38;5;75"},
                                                {"light-magenta", "38;5;207"},
                                                {"light-cyan", "38;5;87"},
                                                {"turquoise", "38;5;80"},
                                                {"violet", "38;5;92"},
                                                {"purple", "38;5;127"},
                                                {"lavender", "38;5;183"},
                                                {"maroon", "38;5;124"},
                                                {"beige", "38;5;230"},
                                                {"olive", "38;5;142"},
                                                {"indigo", "38;5;54"},
                                                {"teal", "38;5;30"},
                                                {"gold", "38;5;220"},
                                                {"silver", "38;5;7"},
                                                {"navy", "38;5;17"},
                                                {"steel", "38;5;188"},
                                                {"salmon", "38;5;173"},
                                                {"peach", "38;5;217"},
                                                {"khaki", "38;5;179"},
                                                {"coral", "38;5;209"},
                                                {"crimson", "38;5;160"}};

void DiscordPage::javaScriptConsoleMessage(
    QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString &message,
    int lineNumber, const QString &sourceID) {
  auto colorSegments = message.split("%c");
  for (auto segment : colorSegments.mid(1)) {
    auto lines = segment.split("\n");
    QString ansi;
    uint endOfStyles = lines.length();
    for (size_t line = 1; line < lines.length(); line++) {
      if (!lines[line].endsWith(";")) {
        endOfStyles = line;
        break;
      }
      if (lines[line] == "font-weight: bold;")
        ansi += "\033[1m";
      else if (lines[line].startsWith("color: ")) {
        auto color = lines[line].mid(7).chopped(1);
        if (cssAnsiColorMap.find(color) != cssAnsiColorMap.end())
          ansi += "\033[" + cssAnsiColorMap[color] + "m";
      }
    }
    qDebug(discordLog) << (ansi + lines[0].trimmed() + "\033[0m " +
                           ((lines.length() > endOfStyles)
                                ? lines[endOfStyles].trimmed()
                                : ""))
                              .toUtf8()
                              .constData();
    for (auto line : lines.mid(endOfStyles + 1)) {
      qDebug(discordLog) << line.toUtf8().constData();
    }
  }
}
