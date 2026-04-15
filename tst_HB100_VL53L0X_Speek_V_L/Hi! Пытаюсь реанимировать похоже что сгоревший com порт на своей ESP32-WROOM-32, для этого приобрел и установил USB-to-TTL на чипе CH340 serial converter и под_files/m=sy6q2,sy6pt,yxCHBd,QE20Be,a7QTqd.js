loaded_h_0(function(_){var window=this;
_.rOq=_.y("JjeCTe",[_.A1a]);
_.iOq=_.y("TPfxRd",[_.A1a]);
_.v("yxCHBd");
var ius=function(a){if(a!==void 0)return{isExpanded:a}},jus=class{constructor(a){this.oa=a}subscribeToExpanderState(a){this.oa.isExpanded.subscribe((b,c)=>{a(ius(b),ius(c))})}},kus=class{constructor(a){this.oa=a}subscribeToInputPlateState(a){this.oa.hasSubmittedQuery.subscribe((b,c)=>{a({hasSubmittedQuery:b},c!==void 0?{hasSubmittedQuery:c}:void 0)})}setHasSubmittedQuery(a){this.oa.hasSubmittedQuery.value=a}subscribeToOverlayState(a){this.oa.rNa.subscribe((b,c)=>{a(b,c)})}setOverlayState(a){this.oa.rNa.value=
a}},lus=function(a,b,c){(b=b.closest(`[jsmodel~='${c}']`))||(b=document.querySelector(`[jsmodel~='${c}']`));if(!b)return null;let d;return((d=a.oa.get(c))==null?void 0:d.get(b))||null};_.$f(_.B1a,class extends _.ko{constructor(){super();this.oa=new Map}Myc(a,b){this.oa.has(a)||this.oa.set(a,new Map);this.oa.get(a).set(b.fda(),b)}getExpanderModel(a){return(a=lus(this,a,_.iOq))?new jus(a):null}getAimOverlayModel(a){return(a=lus(this,a,_.rOq))?new kus(a):null}});
_.x();
_.v("QE20Be");
_.bjr=new _.Vf(_.A1a);
_.x();
_.v("a7QTqd");
_.$f(_.m3a,class extends _.ko{static Ta(){return{service:{OWa:_.bjr}}}constructor(a){super();this.OWa=a.service.OWa}getExpanderModel(a){return this.OWa.getExpanderModel(a)}getAimOverlayModel(a){return this.OWa.getAimOverlayModel(a)}});
_.x();
});
// Google Inc.
