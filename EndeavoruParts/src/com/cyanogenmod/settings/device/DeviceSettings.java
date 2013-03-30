package com.cyanogenmod.settings.device;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.TwoStatePreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;

public class DeviceSettings extends PreferenceActivity  {

    public static final String KEY_S2WSWITCH = "s2w_switch";
    public static final String KEY_S2WSTROKE = "s2w_stroke";
    public static final String KEY_S2WLENGTH = "s2w_length";
    public static final String KEY_BACKLIGHTDISABLE = "backlight_disable";
    public static final String KEY_BACKLIGHTNOTIFICATION = "backlight_notification";
    public static final String KEY_SMARTDIMMERSWITCH = "smartdimmer_switch";
    public static final String KEY_BACKLIGHTBRIGHTNESS = "backlight_brightness";
    public static final String KEY_SLOWBLINKBRIGHTNESSLIMIT = "slow_blink_brightness_limit";

    private TwoStatePreference mS2WSwitch;
    private ListPreference mS2WStroke;
    private ListPreference mS2WLength;
    private TwoStatePreference mBacklightDisable;
    private TwoStatePreference mBacklightNotification;
    private TwoStatePreference mSmartDimmerSwitch;
    private ListPreference mBacklightBrightness;
    private TwoStatePreference mSlowBlinkBrightnessLimit;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.main);

        mS2WSwitch = (TwoStatePreference) findPreference(KEY_S2WSWITCH);
        mS2WSwitch.setEnabled(Sweep2WakeSwitch.isSupported());
        mS2WSwitch.setChecked(Sweep2WakeSwitch.isEnabled(this));
        mS2WSwitch.setOnPreferenceChangeListener(new Sweep2WakeSwitch());

        mS2WStroke = (ListPreference) findPreference(KEY_S2WSTROKE);
        mS2WStroke.setEnabled(Sweep2WakeStroke.isSupported());
        mS2WStroke.setValue(Sweep2WakeStroke.getValue(this));
        mS2WStroke.setOnPreferenceChangeListener(new Sweep2WakeStroke());

        mS2WLength = (ListPreference) findPreference(KEY_S2WLENGTH);
        mS2WLength.setEnabled(Sweep2WakeMinLength.isSupported());
        mS2WLength.setValue(squashLengthValue(Sweep2WakeMinLength.getValue(this)));
        mS2WLength.setOnPreferenceChangeListener(new Sweep2WakeMinLength());

        mBacklightDisable = (TwoStatePreference) findPreference(KEY_BACKLIGHTDISABLE);
        mBacklightDisable.setEnabled(BacklightDisable.isSupported());
        mBacklightDisable.setChecked(BacklightDisable.isEnabled(this)); 
       	mBacklightDisable.setOnPreferenceChangeListener(new BacklightDisable());

        mBacklightNotification = (TwoStatePreference) findPreference(KEY_BACKLIGHTNOTIFICATION);
        mBacklightNotification.setEnabled(BacklightNotificationSwitch.isSupported());
        mBacklightNotification.setChecked(BacklightNotificationSwitch.isEnabled(this));
        mBacklightNotification.setOnPreferenceChangeListener(new BacklightNotificationSwitch());

        mSmartDimmerSwitch = (TwoStatePreference) findPreference(KEY_SMARTDIMMERSWITCH);
        mSmartDimmerSwitch.setEnabled(SmartDimmerSwitch.isSupported());
        mSmartDimmerSwitch.setChecked(SmartDimmerSwitch.isEnabled(this));
        mSmartDimmerSwitch.setOnPreferenceChangeListener(new SmartDimmerSwitch());

        mBacklightBrightness= (ListPreference) findPreference(KEY_BACKLIGHTBRIGHTNESS);
        mBacklightBrightness.setEnabled(BacklightBrightness.isSupported());
        mBacklightBrightness.setValue(squashBrightnessValue(BacklightBrightness.getValue(this)));
        mBacklightBrightness.setOnPreferenceChangeListener(new BacklightBrightness());

        mSlowBlinkBrightnessLimit = (TwoStatePreference) findPreference(KEY_SLOWBLINKBRIGHTNESSLIMIT);
        mSlowBlinkBrightnessLimit.setEnabled(SlowBlinkBrightnessLimit.isSupported());
        mSlowBlinkBrightnessLimit.setChecked(SlowBlinkBrightnessLimit.isEnabled(this));
        mSlowBlinkBrightnessLimit.setOnPreferenceChangeListener(new SlowBlinkBrightnessLimit());
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
    
    private String squashLengthValue(String value) {
        // map it to 325, 500, 850 if not exact value
        int intValue=new Integer(value).intValue();
        if(intValue==325 || intValue==500 || intValue==850){
            return value;
        }
        // we found a different value in sysfs
        // map it to our 3 possible length
        if(intValue<325)
            return "325";
        if(intValue>850)
            return "850";
        if(intValue>325 && intValue <500){
            int diff1=intValue-325;
            int diff2=500-intValue;
            return (diff1<diff2)?"325":"500";
        }
        if(intValue>500 && intValue <850){
            int diff1=intValue-500;
            int diff2=850-intValue;
            return (diff1<diff2)?"500":"850";
        }
        return value;
    } 
    
    private String squashBrightnessValue(String value) {
        int intValue=new Integer(value).intValue();
        if(intValue < 10){
        	return "10";
        }
        if(intValue == 255){
        	return "255";
        }
        return new Integer((intValue/10)*10).toString();
    }
}
