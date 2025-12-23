#!/bin/bash

# Your AppleID, TeamID, Password and Name (An app-specific password NOT! AppleID password)
if [ -z "$APPLE_ID" ]; then
    source notarization.sh
fi

if [ -z "$PROJECT_NAME" ]; then
    PROJECT_NAME=package
fi

# Executable
cp ../build/ppl+ package-root/Applications/HP/Xprime.app/Contents/Resources/Developer/usr/bin/ppl+

# Package
pkgbuild --root package-root \
         --identifier uk.insoft.$PROJECT_NAME \
         --version 1.5 --install-location / \
         $PROJECT_NAME.pkg
         
# Ask the user a question
result=$(osascript -e 'display dialog "Do you want to sign the package?" buttons {"Yes", "No"} default button "Yes"')

# Process the result
if [[ "$result" == *"Yes"* ]]; then
    productsign --sign "Developer ID Installer: $YOUR_NAME ($TEAM_ID)" $PROJECT_NAME.pkg $PROJECT_NAME-signed.pkg

    xcrun notarytool submit --apple-id $APPLE_ID \
                            --password $PASSWORD \
                            --team-id $TEAM_ID \
                            --wait $PROJECT_NAME-signed.pkg
                        
    # Staple
    xcrun stapler staple $PROJECT_NAME-signed.pkg

    # Verify
    xcrun stapler validate $PROJECT_NAME-signed.pkg

# Gatekeeper
    spctl --assess --type install --verbose $PROJECT_NAME-signed.pkg

    rm $PROJECT_NAME.pkg
    mv $PROJECT_NAME-signed.pkg ../ppl+\_Xprime\_$(date +"%Y%m%d").pkg
else
    mv $PROJECT_NAME.pkg ../ppl+\_Xprime\_$(date +"%Y%m%d").pkg
fi
