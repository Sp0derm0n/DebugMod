class ClickButton extends MovieClip
{
    function ClickButton()
    {
		super();
		this._xscale = 100;
		this._yscale = 100;
    }
	
	function HOVER()
    {
		this._xscale = 100;
		this._yscale = 100;
    }
	
	function HIT()
	{
		this._xscale = 80;
		this._yscale = 80;
	}
	
	function ON()
	{
		this._xscale = 100;
		this._yscale = 100;
	}
   
    function OFF()
    {
		this._xscale = 100;
		this._yscale = 100;
    }
	
	function GetWidth():Number
	{
		return this._width;
	}
	
	function GetHeight():Number
	{
		return this._height;
	}
}
