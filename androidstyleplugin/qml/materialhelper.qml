import QtQml 2.12
import QtQuick.Controls.Material 2.12

QtObject {
    function updateMaterialTheme(obj, isDark, fgColor, bgColor, primaryColor, accentColor)
    {
        obj.Material.theme = isDark ? Material.Dark : Material.Light;
        obj.Material.foreground = fgColor;
        obj.Material.background = bgColor;
        obj.Material.primary = primaryColor;
        obj.Material.accent = accentColor;
    }
}
