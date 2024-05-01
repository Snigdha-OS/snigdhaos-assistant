if [ -e "$1" ]; then
	echo ""
	echo "Preparing setup.."
	echo ""
	sudo bash - <$1
fi

echo ""
echo "Installing packages.."
echo ""
installable_packages=$(comm -12 <(pacman -Slq | sort) <(sed s/\\s/\\n/g - <$2 | sort))
sudo pacman -S --needed $installable_packages && rm $2 || { read -p "Error! Press enter to return to Snigdha OS Assistant."; exit; }

if [ -e "$3" ]; then
	echo ""
	echo "Enabling services.."
	echo ""
	sudo bash - <$3
fi

echo ""
read -p "Press enter to return to Snigdha OS Assistant."
