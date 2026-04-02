loaded_h_0(function(_){var window=this;
_.aJq=_.y("JjeCTe",[_.N1a]);
_.SIq=_.y("TPfxRd",[_.N1a]);
_.v("yxCHBd");
var hos=function(a){if(a!==void 0)return{isExpanded:a}},ios=class{constructor(a){this.oa=a}subscribeToExpanderState(a){this.oa.isExpanded.subscribe((b,c)=>{a(hos(b),hos(c))})}},jos=class{constructor(a){this.oa=a}subscribeToInputPlateState(a){this.oa.hasSubmittedQuery.subscribe((b,c)=>{a({hasSubmittedQuery:b},c!==void 0?{hasSubmittedQuery:c}:void 0)})}setHasSubmittedQuery(a){this.oa.hasSubmittedQuery.value=a}subscribeToOverlayState(a){this.oa.tNa.subscribe((b,c)=>{a(b,c)})}setOverlayState(a){this.oa.tNa.value=
a}},kos=function(a,b,c){(b=b.closest(`[jsmodel~='${c}']`))||(b=document.querySelector(`[jsmodel~='${c}']`));if(!b)return null;let d;return((d=a.oa.get(c))==null?void 0:d.get(b))||null};_.Zf(_.O1a,class extends _.ho{constructor(){super();this.oa=new Map}vyc(a,b){this.oa.has(a)||this.oa.set(a,new Map);this.oa.get(a).set(b.gda(),b)}getExpanderModel(a){return(a=kos(this,a,_.SIq))?new ios(a):null}getAimOverlayModel(a){return(a=kos(this,a,_.aJq))?new jos(a):null}});
_.x();
_.v("QE20Be");
_.Kdr=new _.Uf(_.N1a);
_.x();
_.v("a7QTqd");
_.Zf(_.z3a,class extends _.ho{static Ta(){return{service:{NWa:_.Kdr}}}constructor(a){super();this.NWa=a.service.NWa}getExpanderModel(a){return this.NWa.getExpanderModel(a)}getAimOverlayModel(a){return this.NWa.getAimOverlayModel(a)}});
_.x();
});
// Google Inc.
