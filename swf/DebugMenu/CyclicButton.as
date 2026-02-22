class CyclicButton extends ClickButton
{
	// symbol which has all the cyclable icons distributed on layers. top layer is the first child
	var allIcons:MovieClip;
	
	var visibleIndex:Number = 0;
	var numberOfIcons:Number = 0;
	
	function CyclicButton()
	{
		super();
		for (var i in allIcons)
		{
			numberOfIcons += 1;
		}
		SetIndexVisible(0);
	}
	
	function SetIndexVisible(i:Number)
	{
		var k:Number = 0;
		for (var j in allIcons)
		{
			if (i == k)
			{
				allIcons[j]._visible = true;
			}
			else
			{
				allIcons[j]._visible = false;
			}
			k += 1;
			
		}
		visibleIndex = i;
	}
	
	function CycleIcon()
	{
		var nextIndex = visibleIndex + 1;
		if (nextIndex >= numberOfIcons)
		{
			nextIndex = 0;
		}
		SetIndexVisible(nextIndex);
		//if (icon0._visible == true)
		//{
		//	SetIcon1();
		//}
		//else if (icon1._visible == true)
		//{
		//	SetIcon2();
		//}
		//else if (icon2._visible == true)
		//{
		//	SetIcon0();
		//}
	}
	
	function SetIcon0()
	{
		//icon0._visible = true;
		//icon1._visible = false;
		//icon2._visible = false;
	}
	function SetIcon1()
	{
		//icon0._visible = false;
		//icon1._visible = true;
		//icon2._visible = false;
	}
	function SetIcon2()
	{
		//icon0._visible = false;
		//icon1._visible = false;
		//icon2._visible = true;
	}
	
	function GetAllIcons() : String
	{
		var str:String = "All children:";
		for(var i in allIcons)
		{
			var new_str:String = str.concat(" ", "Child:");
			str = new_str;
			if (typeof(allIcons[i]) == "movieclip")
			{
				var new_str:String = str.concat(" ", allIcons[i]._name);
				str = new_str;
				var new_str2:String = str.concat(":", allIcons[i]._visible);
				str = new_str2;
			}
		}
		return str;
	}
	
}
