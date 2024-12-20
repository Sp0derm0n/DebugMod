import flash.geom.ColorTransform;

class ToggleButton extends MovieClip
{
	//var originalColorTransform:ColorTransform;
	
	var onIcon:MovieClip;
	var offIcon:MovieClip;
	
	function ToggleButton()
	{
		super();
		//this.originalColorTransform = this.transform.colorTransform;
		onIcon._visible = false;
		offIcon._visible = true;
		this._xscale = 100;
		this._yscale = 100;
		this._alpha  = 100;
		//this.transform.colorTransform = new ColorTransform(0, 0, 0, 1, 0, 0, 0, 0);
	}
   
	function HOVER()
    {
		onIcon._visible = true;
		offIcon._visible = false;
		this._xscale = 100;
		this._yscale = 100;
		this._alpha  =  60;
		//this.transform.colorTransform = this.originalColorTransform;
    }
   
	function HIT()
	{
		onIcon._visible = true;
		offIcon._visible = false;
		this._xscale = 80;
		this._yscale = 80;
		//this.transform.colorTransform = this.originalColorTransform;
	}
	
	function ON()
	{
		onIcon._visible = true;
		offIcon._visible = false;
		this._xscale = 100;
		this._yscale = 100;
		this._alpha  = 100;
		//this.transform.colorTransform = this.originalColorTransform;
	}
	
    function OFF()
	{
		onIcon._visible = false;
		offIcon._visible = true;
		this._xscale = 100;
		this._yscale = 100;
		this._alpha  = 100;
		//this.transform.colorTransform = new ColorTransform(0, 0, 0, 1, 0, 0, 0, 0);
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
