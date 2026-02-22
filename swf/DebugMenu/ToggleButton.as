class ToggleButton extends MovieClip
{
	
	var onIcon:MovieClip;
	var offIcon:MovieClip;
	var totalScale:Number;
	
	public function ToggleButton()
	{
		super();
		totalScale = 1;

		OFF();
	}
   
	public function HOVER_ON()
    {
		onIcon._visible = true;
		offIcon._visible = false;
		this._xscale = 90*totalScale;
		this._yscale = 90*totalScale;
		this._alpha  = 100;
    }
	
	public function HOVER_OFF()
    {
		onIcon._visible = true;
		offIcon._visible = false;
		this._xscale = 100*totalScale;
		this._yscale = 100*totalScale;
		this._alpha  =  60;
    }
   
	public function HIT()
	{
		onIcon._visible = true;
		offIcon._visible = false;
		this._xscale = 80*totalScale;
		this._yscale = 80*totalScale;
		this._alpha  = 100;
	}
	
	public function ON()
	{
		onIcon._visible = true;
		offIcon._visible = false;
		this._xscale = 100*totalScale;
		this._yscale = 100*totalScale;
		this._alpha  = 100;
	}
	
    public function OFF()
	{
		onIcon._visible = false;
		offIcon._visible = true;
		this._xscale = 100*totalScale;
		this._yscale = 100*totalScale;
		this._alpha  = 100;
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
