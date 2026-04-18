loaded_h_0(function(_){var window=this;
_.OGq=_.y("vL8wte",[_.q1a]);
_.EFq=_.y("JjeCTe",[_.q1a]);
_.BFq=_.y("TPfxRd",[_.q1a]);
_.v("yxCHBd");
var Gns=function(a){if(a!==void 0)return{isExpanded:a}},Hns=class{constructor(a){this.oa=a}subscribeToExpanderState(a){this.oa.isExpanded.subscribe((b,c)=>{a(Gns(b),Gns(c))})}},Ins=class{constructor(a){this.oa=a}subscribeToInputPlateState(a){this.oa.hasSubmittedQuery.subscribe((b,c)=>{a({hasSubmittedQuery:b},c!==void 0?{hasSubmittedQuery:c}:void 0)})}setHasSubmittedQuery(a){this.oa.hasSubmittedQuery.value=a}subscribeToOverlayState(a){this.oa.ANa.subscribe((b,c)=>{a(b,c)})}setOverlayState(a){this.oa.ANa.value=
a}},Jns=class{constructor(a){this.oa=a}subscribeToNonPersonalizedQuerySubmission(a){this.oa.Fhe(a)}},Kns=function(a,b,c){(b=b.closest(`[jsmodel~='${c}']`))||(b=document.querySelector(`[jsmodel~='${c}']`));if(!b)return null;let d;return((d=a.oa.get(c))==null?void 0:d.get(b))||null};
_.Xf(_.r1a,class extends _.jo{constructor(){super();this.oa=new Map}v1b(a,b){this.oa.has(a)||this.oa.set(a,new Map);this.oa.get(a).set(b.hda(),b)}getExpanderModel(a){return(a=Kns(this,a,_.BFq))?new Hns(a):null}getAimOverlayModel(a){return(a=Kns(this,a,_.EFq))?new Ins(a):null}getPersonalizationModel(a){return(a=Kns(this,a,_.OGq))?new Jns(a):null}});
_.x();
_.v("QE20Be");
_.aar=new _.Rf(_.q1a);
_.x();
_.v("a7QTqd");
_.Xf(_.Z2a,class extends _.jo{static Sa(){return{service:{Yxa:_.aar}}}constructor(a){super();this.Yxa=a.service.Yxa}getExpanderModel(a){return this.Yxa.getExpanderModel(a)}getAimOverlayModel(a){return this.Yxa.getAimOverlayModel(a)}getPersonalizationModel(a){return this.Yxa.getPersonalizationModel(a)}});
_.x();
});
// Google Inc.
