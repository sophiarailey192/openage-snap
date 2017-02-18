// Copyright 2017-2017 the openage authors. See copying.md for legal info.

import QtQuick 2.4
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1

import yay.sfttech.font 1.0

import yay.sfttech.openage 1.0

Rectangle {
	id: root

	property SettingsMode settingsMode

	color: "white"

	TabView {
		id: tabView

		anchors.fill: parent
		anchors.topMargin: metricsUnit

		style: tabViewStyle

		Component.onCompleted: {
			currentIndex = root.settingsMode.currentGroupIndex
			root.settingsMode.currentGroupIndex = Qt.binding(function() { return currentIndex })
		}

		Tab {
			title: qsTr("General")
		}

		Tab {
			title: qsTr("Graphics")

			Item {
				anchors.fill: parent
				anchors.margins: 3 * metricsUnit

				GridLayout {

					Text {
						Layout.row: 0
						Layout.column: 0

						text: "Font size:"
					}

					SpinBox {
						Layout.row: 0
						Layout.column: 1

						activeFocusOnPress: false
						minimumValue: 1
						value: GlobalFont.font.pointSize
						onValueChanged: GlobalFont.font.pointSize = value
					}
				}
			}
		}

		Tab {
			title: qsTr("Sound")
		}

		Tab {
			title: qsTr("Mouse")
		}

		Tab {
			title: qsTr("Keyboard")
		}

		Tab {
			title: qsTr("Profiles")
		}

		Tab {
			title: qsTr("Mods")
		}
	}

	Component {
		id: tabViewStyle

		TabViewStyle {
			id: sty

			tabsAlignment: Qt.AlignHCenter

			frameOverlap: 0

			leftCorner: Item {
				implicitHeight: 5 * metricsUnit
			}

			tabBar: Rectangle {
				color: "white"

				Image {
					anchors.left: parent.left
					sourceSize.height: parent.height
					fillMode: Image.PreserveAspectFit
					source: "image://by-filename/logo/banner.png.0"
				}
			}

			tab: Rectangle {
				property int totalOverlap: sty.tabOverlap * (control.count - 1)
				property real maxTabWidth: control.count > 0 ? (styleData.availableWidth + totalOverlap) / control.count : 0

				implicitWidth: Math.round(Math.min(maxTabWidth, textItem.implicitWidth + 5 * metricsUnit))
				implicitHeight: 7 * metricsUnit

				color: "transparent"

				Rectangle {
					anchors.fill: parent
					anchors.left: parent.left
					anchors.right: parent.right
					anchors.bottom: parent.bottom
					anchors.topMargin: 1.5 * metricsUnit
					anchors.bottomMargin: -1

					Text {
						id: textItem

						anchors.fill: parent
						anchors.leftMargin: 5 * metricsUnit
						anchors.rightMargin: 5 * metricsUnit
						anchors.topMargin: 5 * metricsUnit
						anchors.bottomMargin: 5 * metricsUnit
						verticalAlignment: Text.AlignVCenter
						horizontalAlignment: Text.AlignHCenter

						text: styleData.title
						font.bold: styleData.selected
					}

					color: "white"
					border.color: "black"
				}
			}

			frame: Rectangle {
				color: "white"
				border.color: "black"
			}
		}
	}

	/**
	 * Metric propagation.
	 */
	FontMetrics {
		id: fontMetrics
	}

	property int metricsUnit: metrics ? metrics.unit : fontMetrics.averageCharacterWidth
}
