#pragma once
#include <QJsonObject>
#include <QSettings>
#include <memory>
#include "i_settings_service.hpp"

namespace application::services
{

class SettingsService : public ISettingsService
{
    Q_OBJECT

public:
    SettingsService();

    QString getSetting(const QString& settingName) override;
    void setSetting(const QString& settingName, const QString& value) override;

public slots:
    void loadUserSettings(const QString& token, const QString& email) override;
    void clearUserData() override;

private:
    void createSettings();
    QString getUniqueUserHash() const;
    void generateDefaultSettings();
    QJsonObject getDefaultSettings();

    std::unique_ptr<QSettings> m_settings;
    QString m_userEmail;
    QString m_defaultSettingsFile = ":/resources/data/default_settings.json";
};

}  // namespace application::services