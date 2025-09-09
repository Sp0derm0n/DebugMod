import flash.geom.ColorTransform;

class MenuContainer extends MovieClip
{
	//var originalColorTransform:ColorTransform;
	
	function MenuContainer()
	{
		super();
		//this.originalColorTransform = this.transform.colorTransform;
		this._xscale = 100;
		this._yscale = 100;
		this._alpha  = 100;
		//this.transform.colorTransform = new ColorTransform(0, 0, 0, 1, 0, 0, 0, 0);
	}
	
	function Hide()
	{
		this._visible = false;
	}
	
	function Show()
	{
		this._visible = true;
	}
	
	function GetWidth():Number
	{
		return this._width;
	}
	
	function GetHeight():Number
	{
		return this._height;
	}
	
	function SetY(a_y:Number)
	{
		this._y = a_y;
	}
}
