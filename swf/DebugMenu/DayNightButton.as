class DayNightButton extends ClickButton
{
	var sunIcon:MovieClip;
	var moonIcon:MovieClip;
	var autoIcon:MovieClip;
	
	function DayNightButton()
	{
		super();
		moonIcon._visible = false;
		autoIcon._visible = false;
	}
	
	function CycleIcon()
	{
		if (sunIcon._visible == true)
		{
			SetMoonIcon();
		}
		else if (moonIcon._visible == true)
		{
			SetAutoIcon();
		}
		else if (autoIcon._visible == true)
		{
			SetSunIcon();
		}
	}
	
	function SetSunIcon()
	{
		sunIcon._visible = true;
		moonIcon._visible = false;
		autoIcon._visible = false;
	}
	function SetMoonIcon()
	{
		sunIcon._visible = false;
		moonIcon._visible = true;
		autoIcon._visible = false;
	}
	function SetAutoIcon()
	{
		sunIcon._visible = false;
		moonIcon._visible = false;
		autoIcon._visible = true;
	}
	
}
