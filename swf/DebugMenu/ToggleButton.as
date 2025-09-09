import flash.geom.ColorTransform;

class ToggleButton extends MovieClip
{
	//var originalColorTransform:ColorTransform;
	
	var onIcon:MovieClip;
	var offIcon:MovieClip;
	var totalScale:Number;
	
	public function ToggleButton()
	{
		super();
		totalScale = 1;

		//this.originalColorTransform = this.transform.colorTransform;
		onIcon._visible = false;
		offIcon._visible = true;
		this._xscale = 100*totalScale;
		this._yscale = 100*totalScale;
		this._alpha  = 100;
		//this.transform.colorTransform = new ColorTransform(0, 0, 0, 1, 0, 0, 0, 0);
	}
   
	public function HOVER()
    {
		onIcon._visible = true;
		offIcon._visible = false;
		this._xscale = 100*totalScale;
		this._yscale = 100*totalScale;
		this._alpha  =  60;
		//this.transform.colorTransform = this.originalColorTransform;
    }
   
	public function HIT()
	{
		onIcon._visible = true;
		offIcon._visible = false;
		this._xscale = 80*totalScale;
		this._yscale = 80*totalScale;
		//this.transform.colorTransform = this.originalColorTransform;
	}
	
	public function ON()
	{
		onIcon._visible = true;
		offIcon._visible = false;
		this._xscale = 100*totalScale;
		this._yscale = 100*totalScale;
		this._alpha  = 100;
		//this.transform.colorTransform = this.originalColorTransform;
	}
	
    public function OFF()
	{
		onIcon._visible = false;
		offIcon._visible = true;
		this._xscale = 100*totalScale;
		this._yscale = 100*totalScale;
		this._alpha  = 100;
		//this.transform.colorTransform = new ColorTransform(0, 0, 0, 1, 0, 0, 0, 0);
	}
	
	public function GetWidth():Number
	{
		return this._width;
	}
	
	public function GetHeight():Number
	{
		return this._height;
	}
	
	public function CreateClone(a_name:String)
	{
		this.duplicateMovieClip(a_name, 0); // 0 = depth
	}
	
	public function SetPosition(a_x:Number, a_y:Number)
	{
		this._x = a_x;
		this._y = a_y;
	}
	
	public function SetScale(a_scale:Number) //between 0 and 1 plz
	{
		totalScale = a_scale;
		this._xscale = 100*a_scale;
		this._yscale = 100*a_scale;
	}
	
	public function Hide()
	{
		this._visible = false;
	}
}
